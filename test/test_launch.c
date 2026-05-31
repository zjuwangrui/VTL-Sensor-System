
/*

  */
#include "stm32f10x.h" 
#include "stm32f1xx_hal.h"

/**
  *   
  */
int main(void)
{	

	RCC_APB2ENR |= (1<<3);


	GPIOB_CRL &= ~( 0x0F<< (4*0));	

	GPIOB_CRL |= (1<<4*0);


	GPIOB_ODR &=(1<<0);
	
	while(1);
}


