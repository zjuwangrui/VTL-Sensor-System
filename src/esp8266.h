#ifndef ESP8266_H
#define ESP8266_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* ===== 硬件绑定 ===== */
// ESP8266 接在 USART2: PA2(TX) PA3(RX)，在CubeMX中开启USART2并使能全局中断
extern UART_HandleTypeDef huart2;
#define ESP_UART huart2

/* ===== 网络配置 ===== */
#define AP_SSID      "STM32_Sensor"   // 与React Native APP中一致
#define AP_PASS      "12345678"
#define PHONE_IP     "192.168.4.2"    // 手机连上AP后通常获得此IP（APP界面有显示）
#define PHONE_PORT   8080

/* ===== 超时（ms） ===== */
#define ESP_TIMEOUT_SHORT  2000U
#define ESP_TIMEOUT_LONG   6000U

/* ===== 返回值 ===== */
typedef enum {
    ESP_OK = 0,
    ESP_ERR_TIMEOUT,
    ESP_ERR_AT,
} ESP_Result;

/* ===== 状态快照（供main.c读取显示） ===== */
typedef struct {
    bool ap_ready;          // AP已建立
    bool last_send_ok;      // 上次发送成功
    uint32_t send_count;    // 累计发送次数
    uint32_t fail_count;    // 累计失败次数
} ESP_Stats;

extern ESP_Stats g_esp_stats;

/* ===== 接口 ===== */

/**
 * 初始化ESP8266: 复位 → 设AP模式 → 建热点 → 单连接模式
 * 在main()中调用一次
 */
ESP_Result ESP_Init(void);

/**
 * 向手机POST一次电压数据，同时取回新阈值
 * @param voltage    当前ADC电压（V）
 * @param led_on     当前LED状态
 * @param out_thr    [out] 手机下发的阈值，失败时不修改
 * @return ESP_OK 表示通信成功
 */
ESP_Result ESP_PostSensor(float voltage, bool led_on, float *out_thr);

/**
 * UART接收中断回调中调用此函数
 * 在 HAL_UART_RxCpltCallback 里转发
 */
void ESP_UART_IRQHandler(void);

#endif /* ESP8266_H */
