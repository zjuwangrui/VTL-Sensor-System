/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2016-xx-xx
  * @brief   �����������ӿ�
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  STM32 F103 ������  
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_beep.h"   

 /**
  * @brief  ��ʼ�����Ʒ�������IO
  * @param  ��
  * @retval ��
  */
void BEEP_GPIO_Config(void)
{		
    /*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*�������Ʒ�������GPIO�Ķ˿�ʱ��*/
    BEEP_GPIO_CLK_ENABLE();   

    /*ѡ��Ҫ���Ʒ�������GPIO*/															   
    GPIO_InitStructure.Pin = BEEP_GPIO_PIN;	

    /*����GPIOģʽΪͨ���������*/
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;   

    GPIO_InitStructure.Pull = GPIO_PULLDOWN;

    /*����GPIO����Ϊ50MHz */   
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 

    /*���ÿ⺯������ʼ�����Ʒ�������GPIO*/
    HAL_GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);			 

    /*  �رշ�����*/
    HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN,GPIO_PIN_RESET);	 
}
/*********************************************END OF FILE**********************/
