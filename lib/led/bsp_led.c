/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   ledӦ�ú����ӿ�
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� STM32 F103 ������  
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_led.h"   

 /**
  * @brief  ��ʼ������LED��IO
  * @param  ��
  * @retval ��
  */
void LED_GPIO_Config(void)
{		
		
    /*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
    GPIO_InitTypeDef  GPIO_InitStruct;

    /*����LED��ص�GPIO����ʱ��*/
    LED1_GPIO_CLK_ENABLE();
    LED2_GPIO_CLK_ENABLE();
    LED3_GPIO_CLK_ENABLE();
  
    /*ѡ��Ҫ���Ƶ�GPIO����*/															   
    GPIO_InitStruct.Pin = LED1_PIN;	

    /*�������ŵ��������Ϊ�������*/
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  

    /*��������Ϊ����ģʽ*/
    GPIO_InitStruct.Pull  = GPIO_PULLUP;

    /*������������Ϊ���� */   
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /*���ÿ⺯����ʹ���������õ�GPIO_InitStructure��ʼ��GPIO*/
    HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);	

    /*ѡ��Ҫ���Ƶ�GPIO����*/															   
    GPIO_InitStruct.Pin = LED2_PIN;	
    HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);	

    /*ѡ��Ҫ���Ƶ�GPIO����*/															   
    GPIO_InitStruct.Pin = LED3_PIN;	
    HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);	

    /*�ر�RGB��*/
    LED_RGBOFF;
}



/*********************************************END OF FILE**********************/
