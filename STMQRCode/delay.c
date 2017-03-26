#include "stm32f4xx.h"
#include "delay.h"


static uint8_t  fac_us = 0;

void delay_init( uint8_t SYSCLK )
{
	SysTick->CTRL &= 0xFFFFFFFB;
	fac_us = SYSCLK / 8;
}								    

void delay_ms( uint16_t ms )
{
	while( ms-- )
	     delay_us( 999 );
}   

void delay_us( uint32_t us )
{		
	uint32_t temp;
	SysTick->LOAD = us * fac_us;
	SysTick->VAL = 0x00;
	SysTick->CTRL = 0x01;
	do
	{
		temp = SysTick->CTRL;
	}
	while( temp & 0x01 &&!( temp & ( 1 << 16 )));
	SysTick->CTRL = 0x00;
	SysTick->VAL = 0x00;
}  

