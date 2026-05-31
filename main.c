/**
 * main.c — VTL Sensor System (STM32F103VE 野火指南者)
 *
 * 引脚说明（对应CubeMX配置）:
 *   PA0  → ADC1_IN0   被测电压输入（0~3.3V，注意不要超压）
 *   PA2  → USART2_TX  连 ESP8266 RX
 *   PA3  → USART2_RX  连 ESP8266 TX
 *   PB5  → GPIO_OUT   LED_RED（野火指南者板载红色LED，低电平亮）
 *   PB0  → GPIO_OUT   LED_GREEN（可选，高阈值指示）
 *   PA9  → USART1_TX  调试串口（ST-Link，波特率115200）
 *
 * CubeMX必须开启:
 *   ADC1  IN0  独立模式  软件触发
 *   USART1 115200 8N1 Async（调试，可选）
 *   USART2 115200 8N1 Async  + 开启全局中断
 *   GPIOB PB5 PB0 Output_PP
 *   时钟:  内外晶振均可，推荐72MHz
 */

/* USER CODE BEGIN Includes */
#include "esp8266.h"
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* ===== 外设句柄（CubeMX生成） ===== */
extern ADC_HandleTypeDef  hadc1;
extern UART_HandleTypeDef huart1; // 调试口
extern UART_HandleTypeDef huart2; // ESP8266

/* ===== LED引脚（野火指南者，低电平亮） ===== */
#define LED_RED_PORT   GPIOB
#define LED_RED_PIN    GPIO_PIN_5
#define LED_GREEN_PORT GPIOB
#define LED_GREEN_PIN  GPIO_PIN_0

#define LED_ON(port, pin)   HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define LED_OFF(port, pin)  HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)

/* ===== 调试打印（通过USART1发到PC，可用串口助手查看） ===== */
#ifdef DEBUG
  #define DBG(fmt, ...) do { \
      char _buf[128]; \
      snprintf(_buf, sizeof(_buf), fmt "\r\n", ##__VA_ARGS__); \
      HAL_UART_Transmit(&huart1, (uint8_t *)_buf, strlen(_buf), 100); \
  } while (0)
#else
  #define DBG(fmt, ...) ((void)0)
#endif

/* ===== ADC读取 ===== */
static float ADC_ReadVoltage(void)
{
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) {
        HAL_ADC_Stop(&hadc1);
        return 0.0f;
    }
    uint32_t raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    // 12位ADC，参考电压3.3V
    return (float)raw / 4095.0f * 3.3f;
}

/* ===== 主函数 ===== */
int main(void)
{
    /* --- CubeMX生成的初始化（保留原有内容） --- */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART1_UART_Init(); // 调试口（可选）
    MX_USART2_UART_Init(); // ESP8266

    /* --- 上电状态：两个LED全灭 --- */
    LED_OFF(LED_RED_PORT,   LED_RED_PIN);
    LED_OFF(LED_GREEN_PORT, LED_GREEN_PIN);

    /* --- 初始化ESP8266（阻塞，最长约15s） --- */
    DBG("ESP8266 initializing...");
    ESP_Result init_result;
    uint8_t retry = 0;
    do {
        init_result = ESP_Init();
        if (init_result == ESP_OK) break;
        DBG("ESP init failed, retry %d", retry);
        HAL_Delay(2000);
    } while (++retry < 3);

    if (init_result == ESP_OK) {
        DBG("ESP8266 AP ready: SSID=%s", AP_SSID);
        // 用绿灯闪烁3次表示ESP初始化成功
        for (int i = 0; i < 3; i++) {
            LED_ON(LED_GREEN_PORT, LED_GREEN_PIN);
            HAL_Delay(200);
            LED_OFF(LED_GREEN_PORT, LED_GREEN_PIN);
            HAL_Delay(200);
        }
    } else {
        // ESP初始化失败：红灯快闪
        DBG("ESP8266 init FAILED");
        for (;;) {
            LED_ON(LED_RED_PORT, LED_RED_PIN);
            HAL_Delay(100);
            LED_OFF(LED_RED_PORT, LED_RED_PIN);
            HAL_Delay(100);
        }
    }

    /* ===== 主循环变量 ===== */
    float voltage   = 0.0f;
    float threshold = 1.65f;  // 默认阈值，手机下发后更新
    bool  led_on    = false;
    uint32_t last_send_tick = 0;
    const uint32_t SEND_INTERVAL_MS = 1000; // 每秒发送一次

    /* ===== 主循环 ===== */
    while (1)
    {
        /* 1. 读ADC电压 */
        voltage = ADC_ReadVoltage();

        /* 2. 与阈值比较，驱动LED */
        bool new_led = (voltage > threshold);
        if (new_led != led_on) {
            led_on = new_led;
            if (led_on) {
                LED_ON(LED_RED_PORT, LED_RED_PIN);
                LED_OFF(LED_GREEN_PORT, LED_GREEN_PIN);
                DBG("LED ON  V=%.3f > THR=%.3f", voltage, threshold);
            } else {
                LED_OFF(LED_RED_PORT, LED_RED_PIN);
                LED_ON(LED_GREEN_PORT, LED_GREEN_PIN);
                DBG("LED OFF V=%.3f <= THR=%.3f", voltage, threshold);
            }
        }

        /* 3. 定时向手机发送数据，取回阈值 */
        if (HAL_GetTick() - last_send_tick >= SEND_INTERVAL_MS) {
            last_send_tick = HAL_GetTick();

            float new_thr = threshold; // 若通信失败保持原值
            ESP_Result r = ESP_PostSensor(voltage, led_on, &new_thr);

            if (r == ESP_OK) {
                threshold = new_thr;
                DBG("POST OK  V=%.3f LED=%d THR=%.3f", voltage, led_on, threshold);
            } else {
                DBG("POST FAIL (code=%d) V=%.3f", r, voltage);
                // 通信失败：红灯短闪一次作为提示（不影响LED逻辑）
                uint8_t old_led_state = HAL_GPIO_ReadPin(LED_RED_PORT, LED_RED_PIN);
                LED_ON(LED_RED_PORT, LED_RED_PIN);
                HAL_Delay(50);
                HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, (GPIO_PinState)old_led_state);
            }
        }

        /* 不需要delay，ADC+UART已经消耗了足够时间 */
    }
}

/* ===== UART中断回调（在usart.c或此文件末尾放置） ===== */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        ESP_UART_IRQHandler();
    }
}
