#include "stm32f1xx_hal.h"
#include <string.h>
UART_HandleTypeDef huart1;  // 串口句柄

// 函数声明
static void MX_USART1_UART_Init(void);
static void SystemClock_Config(void);

int main(void)
{
    // 初始化HAL库
    HAL_Init();
    
    // 配置系统时钟
    SystemClock_Config();
    
    // 初始化串口
    MX_USART1_UART_Init();
    
    // 要发送的数据
    char msg1[] = "STM32F103 UART Test\r\n";
    char msg2[] = "Hello, World!\r\n";
    uint8_t tx_buffer[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    
    while (1)
    {
        // 发送字符串
        HAL_UART_Transmit(&huart1, (uint8_t*)msg1, strlen(msg1), 100);
        HAL_Delay(1000);
        
        // 发送字符串
        HAL_UART_Transmit(&huart1, (uint8_t*)msg2, strlen(msg2), 100);
        HAL_Delay(1000);
        
        // 发送十六进制数据
        HAL_UART_Transmit(&huart1, tx_buffer, sizeof(tx_buffer), 100);
        HAL_Delay(1000);
    }
}

// 串口1初始化 (PA9-TX, PA10-RX)
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;           // 波特率
    huart1.Init.WordLength = UART_WORDLENGTH_8B;  // 数据位8位
    huart1.Init.StopBits = UART_STOPBITS_1;       // 停止位1位
    huart1.Init.Parity = UART_PARITY_NONE;        // 无校验
    huart1.Init.Mode = UART_MODE_TX_RX;           // 发送和接收模式
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;  // 无硬件流控制
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;  // 16倍过采样
    
    HAL_UART_Init(&huart1);
}

// 系统时钟配置 (72MHz)
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // 配置HSE晶振 (外部8MHz晶振)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  // 8MHz * 9 = 72MHz
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    
    // 配置系统时钟、AHB、APB时钟
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;      // HCLK = SYSCLK
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;       // APB1 = HCLK/2 = 36MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;       // APB2 = HCLK = 72MHz
    
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

// 串口初始化回调
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(uartHandle->Instance == USART1)
    {
        // 使能USART1时钟
        __HAL_RCC_USART1_CLK_ENABLE();
        // 使能GPIOA时钟
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        // 配置PA9为TX (复用推挽输出)
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        // 配置PA10为RX (浮空输入)
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

