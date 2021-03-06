/*
* Timer setup code
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stm32f4xx.h>
#include <stm32f4xx_tim.h>
#include "serial.h"
#include "gps.h"
#include "gpio.h"

static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static char latlong[29]; /* max 28 bytes to send */
static uint32_t numTransmits;

void Timer_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the TIM2 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	 
	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 5000 - 1; //20000 - 1; // 1 MHz down to 1 KHz (1 ms)
	TIM_TimeBaseStructure.TIM_Prescaler = 42000 - 1; // 24 MHz Clock down to 1 MHz (adjust per your clock)
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	/* TIM IT enable */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
	
	numTransmits = 0;
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		/* Interrupt handler code here: */
		USART1_DMAsends("\r\nTimer tick!\r\n");
		//USART_puts(USART6, "\r\nTimer tick! USART6\r\n");
		/* PARSE NMEA HERE */
		if(ParseNMEA()==0)
		{
			snprintf(latlong, 29, "                               "); /* Fill with blank spaces */
			GPIO_Fix(); /* Status LED stays on */
			/* Log lat/long to database */
			if(numTransmits < 5)
			{
				snprintf(latlong, 29, "POSTy%011f#%011f", GetLat(), GetLong());
				numTransmits++;
			}
			else
			{
				snprintf(latlong, 29, "POST%011f#%012f", GetLat(), GetLong());
			}
			
			USART_puts(USART6, latlong);
		}
		else
			GPIO_NoFix(); /* Status LED can flash */
	}
}
