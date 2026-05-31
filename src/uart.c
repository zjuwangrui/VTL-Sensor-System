#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

UART_HandleTypeDef huart1;

static char _printf_buf[UART_PRINTF_BUF_SZ];

void UART_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    /* PA9 — TX, alternate function push-pull */
    gpio.Pin   = UART_TX_PIN;
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(UART_GPIO_PORT, &gpio);

    /* PA10 — RX, input floating */
    gpio.Pin  = UART_RX_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(UART_GPIO_PORT, &gpio);

    huart1.Instance          = UART_INSTANCE;
    huart1.Init.BaudRate     = UART_BAUDRATE;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
        while (1);
}

void UART_SendByte(uint8_t byte)
{
    HAL_UART_Transmit(&huart1, &byte, 1, HAL_MAX_DELAY);
}

void UART_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (const uint8_t *)str, (uint16_t)strlen(str), HAL_MAX_DELAY);
}

void UART_SendBytes(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, data, len, HAL_MAX_DELAY);
}

int UART_Printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(_printf_buf, sizeof(_printf_buf), fmt, args);
    va_end(args);

    if (len > 0)
        UART_SendBytes((const uint8_t *)_printf_buf, (uint16_t)len);

    return len;
}

uint8_t UART_ReceiveByte(uint32_t timeout_ms)
{
    uint8_t byte = 0;
    HAL_UART_Receive(&huart1, &byte, 1, timeout_ms);
    return byte;
}

HAL_StatusTypeDef UART_ReceiveBytes(uint8_t *buf, uint16_t len, uint32_t timeout_ms)
{
    return HAL_UART_Receive(&huart1, buf, len, timeout_ms);
}
