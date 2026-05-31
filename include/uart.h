#ifndef __UART_H
#define __UART_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define UART_INSTANCE       USART1
#define UART_BAUDRATE       115200U
#define UART_TX_PIN         GPIO_PIN_9
#define UART_RX_PIN         GPIO_PIN_10
#define UART_GPIO_PORT      GPIOA
#define UART_PRINTF_BUF_SZ  256U

extern UART_HandleTypeDef huart1;

void             UART_Init(void);
void             UART_SendByte(uint8_t byte);
void             UART_SendString(const char *str);
void             UART_SendBytes(const uint8_t *data, uint16_t len);
int              UART_Printf(const char *fmt, ...);
uint8_t          UART_ReceiveByte(uint32_t timeout_ms);
HAL_StatusTypeDef UART_ReceiveBytes(uint8_t *buf, uint16_t len, uint32_t timeout_ms);

#endif /* __UART_H */
