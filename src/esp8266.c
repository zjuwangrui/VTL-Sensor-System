#include "esp8266.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ===== 内部接收环形缓冲区 ===== */
#define RX_BUF_SIZE 1024U

static uint8_t  rx_ring[RX_BUF_SIZE];
static volatile uint32_t rx_head = 0; // 写指针（中断写入）
static volatile uint32_t rx_tail = 0; // 读指针（主循环读取）
static uint8_t  rx_byte_isr;          // 单字节DMA目标

ESP_Stats g_esp_stats = {0};

/* ===== 中断接口 ===== */
void ESP_UART_IRQHandler(void)
{
    rx_ring[rx_head % RX_BUF_SIZE] = rx_byte_isr;
    rx_head++;
    HAL_UART_Receive_IT(&ESP_UART, &rx_byte_isr, 1);
}

/* ===== 内部工具函数 ===== */

static void rx_clear(void)
{
    rx_tail = rx_head;
}

static uint32_t rx_available(void)
{
    return rx_head - rx_tail;
}

static uint8_t rx_pop(void)
{
    return rx_ring[(rx_tail++) % RX_BUF_SIZE];
}

static void esp_send_raw(const char *s)
{
    HAL_UART_Transmit(&ESP_UART, (const uint8_t *)s, strlen(s), 2000);
}

/**
 * 等待 buf 中出现 expect 字符串，超时返回 ESP_ERR_TIMEOUT
 * 期间如出现 "ERROR" 或 "FAIL" 立即返回 ESP_ERR_AT
 */
static ESP_Result esp_wait_for(const char *expect, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    char acc[RX_BUF_SIZE];
    uint32_t idx = 0;

    while (HAL_GetTick() - start < timeout_ms) {
        while (rx_available() > 0 && idx < sizeof(acc) - 1) {
            acc[idx++] = (char)rx_pop();
            acc[idx]   = '\0';
        }
        if (strstr(acc, expect))       return ESP_OK;
        if (strstr(acc, "ERROR"))      return ESP_ERR_AT;
        if (strstr(acc, "FAIL"))       return ESP_ERR_AT;
    }
    return ESP_ERR_TIMEOUT;
}

/**
 * 发送 AT 命令并等待期望响应
 * 自动补\r\n
 */
static ESP_Result esp_cmd(const char *cmd, const char *expect, uint32_t timeout_ms)
{
    rx_clear();
    esp_send_raw(cmd);
    esp_send_raw("\r\n");
    return esp_wait_for(expect, timeout_ms);
}

/**
 * 继续读取数据直到超过 idle_ms 没有新数据（或缓冲满）
 * 用于读取HTTP响应
 */
static void esp_read_until_idle(char *buf, uint32_t buf_size, uint32_t max_wait_ms, uint32_t idle_ms)
{
    uint32_t total_start = HAL_GetTick();
    uint32_t last_rx     = HAL_GetTick();
    uint32_t idx = 0;

    while (HAL_GetTick() - total_start < max_wait_ms && idx < buf_size - 1) {
        if (rx_available() > 0) {
            buf[idx++] = (char)rx_pop();
            buf[idx]   = '\0';
            last_rx = HAL_GetTick();
        } else if (idx > 0 && HAL_GetTick() - last_rx > idle_ms) {
            break; // 有数据且空闲超过idle_ms，认为接收完毕
        }
    }
}

/* ===== 公开接口实现 ===== */

ESP_Result ESP_Init(void)
{
    // 启动中断接收
    HAL_UART_Receive_IT(&ESP_UART, &rx_byte_isr, 1);
    HAL_Delay(500);

    // 测试通信，若失败先复位
    if (esp_cmd("AT", "OK", ESP_TIMEOUT_SHORT) != ESP_OK) {
        rx_clear();
        esp_send_raw("AT+RST\r\n");
        HAL_Delay(2000);
        rx_clear();
        if (esp_cmd("AT", "OK", ESP_TIMEOUT_SHORT) != ESP_OK) {
            return ESP_ERR_AT;
        }
    }

    // 关闭回显，减少噪声
    esp_cmd("ATE0", "OK", ESP_TIMEOUT_SHORT);

    // 设置AP模式
    if (esp_cmd("AT+CWMODE=2", "OK", ESP_TIMEOUT_SHORT) != ESP_OK)
        return ESP_ERR_AT;

    // 建立热点
    char cwsap_cmd[96];
    snprintf(cwsap_cmd, sizeof(cwsap_cmd),
             "AT+CWSAP=\"%s\",\"%s\",6,3", AP_SSID, AP_PASS);
    if (esp_cmd(cwsap_cmd, "OK", ESP_TIMEOUT_LONG) != ESP_OK)
        return ESP_ERR_AT;

    // 单连接模式（STM32做客户端，每次POST后关闭）
    if (esp_cmd("AT+CIPMUX=0", "OK", ESP_TIMEOUT_SHORT) != ESP_OK)
        return ESP_ERR_AT;

    g_esp_stats.ap_ready = true;
    return ESP_OK;
}

ESP_Result ESP_PostSensor(float voltage, bool led_on, float *out_thr)
{
    char body[48];
    char request[256];
    char cmd_buf[48];
    char response[768];

    // 构造JSON body
    snprintf(body, sizeof(body), "{\"v\":%.3f,\"led\":%d}", voltage, led_on ? 1 : 0);
    int body_len = (int)strlen(body);

    // 构造HTTP POST请求
    int req_len = snprintf(request, sizeof(request),
        "POST /sensor HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s",
        PHONE_IP, body_len, body);

    /* 1. 建立TCP连接 */
    snprintf(cmd_buf, sizeof(cmd_buf),
             "AT+CIPSTART=\"TCP\",\"%s\",%d", PHONE_IP, PHONE_PORT);
    rx_clear();
    esp_send_raw(cmd_buf);
    esp_send_raw("\r\n");
    // 等待 CONNECT 或 OK
    if (esp_wait_for("CONNECT", ESP_TIMEOUT_SHORT) != ESP_OK) {
        esp_cmd("AT+CIPCLOSE", "OK", 1000);
        g_esp_stats.fail_count++;
        g_esp_stats.last_send_ok = false;
        return ESP_ERR_TIMEOUT;
    }

    /* 2. 告知发送长度，等待 > 提示符 */
    rx_clear();
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+CIPSEND=%d", req_len);
    esp_send_raw(cmd_buf);
    esp_send_raw("\r\n");
    if (esp_wait_for(">", ESP_TIMEOUT_SHORT) != ESP_OK) {
        esp_cmd("AT+CIPCLOSE", "OK", 1000);
        g_esp_stats.fail_count++;
        g_esp_stats.last_send_ok = false;
        return ESP_ERR_TIMEOUT;
    }

    /* 3. 发送HTTP请求体 */
    rx_clear();
    HAL_UART_Transmit(&ESP_UART, (const uint8_t *)request, req_len, 2000);

    /* 4. 等待 SEND OK 确认 */
    if (esp_wait_for("SEND OK", 3000) != ESP_OK) {
        esp_cmd("AT+CIPCLOSE", "OK", 1000);
        g_esp_stats.fail_count++;
        g_esp_stats.last_send_ok = false;
        return ESP_ERR_TIMEOUT;
    }

    /* 5. 等待 +IPD（表示有响应数据到来）*/
    if (esp_wait_for("+IPD", 5000) != ESP_OK) {
        esp_cmd("AT+CIPCLOSE", "OK", 1000);
        g_esp_stats.fail_count++;
        g_esp_stats.last_send_ok = false;
        return ESP_ERR_TIMEOUT;
    }

    /* 6. 读取完整响应（等待200ms空闲） */
    memset(response, 0, sizeof(response));
    esp_read_until_idle(response, sizeof(response), 3000, 200);

    /* 7. 关闭连接 */
    esp_cmd("AT+CIPCLOSE", "OK", 1000);

    /* 8. 从HTTP响应中解析JSON body: {"thr":1.500} */
    char *json_start = strchr(response, '{');
    if (json_start && out_thr) {
        char *thr_key = strstr(json_start, "\"thr\":");
        if (thr_key) {
            float val = strtof(thr_key + 6, NULL);
            if (val > 0.0f && val <= 3.3f) {
                *out_thr = val;
            }
        }
    }

    g_esp_stats.send_count++;
    g_esp_stats.last_send_ok = true;
    return ESP_OK;
}
