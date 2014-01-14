/* USART serial code
* used by GPS, GPRS comms
* maintained by Thomas Payne tp8g10
*/

#include <stm32f4xx.h>
#include <misc.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_dma.h>
#include <stdio.h>
#include <string.h>
#include "serial.h"
#include "globals.h"

/*
* PINS
* GPS: USART1 PB6 Tx, PB7 Rx
* USB: USART1 PA9 Tx, PA10 Rx
*/

/* Private variables */
char received_string[MAX_STRLEN]; /* String received over USART */
char nmea_data[MAX_GPS_STRLEN]; /* GPS nmea data, before lat/long extracted */
static int timeout = 10000;
static char Buffer[MAXUSARTBUF];
static uint8_t uart1virgin=1; /* no virgins were used in the making of this function */
static DMA_InitTypeDef  DMA_InitStructure;

/* Private Functions */
int8_t USART_init(USART_TypeDef* USARTx, uint32_t baudrate);
void init_USART1(uint32_t baudrate);

/* Initialisation function for serial peripherals */
int8_t Serial_init(void)
{
	if( USART_init(USART1, 9600) != 0 ) /* USART1, USB */
		return -1; /* USART init failed */
	
	//init_USART1(9600);

	return 0;
}

void USART_puts(USART_TypeDef* USARTx, volatile char *s){

	while(*s){
		// wait until data register is empty
		while( !(USARTx->SR & 0x00000040) );
		USART_SendData(USARTx, *s);
		*s++;
	}
}

void init_USART1(uint32_t baudrate){

	/* This is a concept that has to do with the libraries provided by ST
	 * to make development easier the have made up something similar to
	 * classes, called TypeDefs, which actually just define the common
	 * parameters that every peripheral needs to work correctly
	 *
	 * They make our life easier because we don't have to mess around with
	 * the low level stuff of setting bits in the correct registers
	 */
	GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
	USART_InitTypeDef USART_InitStruct; // this is for the USART1 initilization
	NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)

	/* enable APB2 peripheral clock for USART1
	 * note that only USART1 and USART6 are connected to APB2
	 * the other USARTs are connected to APB1
	 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* enable the peripheral clock for the pins used by
	 * USART1, PB6 for TX and PB7 for RX
	 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* This sequence sets up the TX and RX pins
	 * so they work correctly with the USART1 peripheral
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins 6 (TX) and 7 (RX) are used
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; 			// the pins are configured as alternate function so the USART peripheral has access to them
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		// this defines the IO speed and has nothing to do with the baudrate!
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			// this defines the output type as push pull mode (as opposed to open drain)
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			// this activates the pullup resistors on the IO pins
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// now all the values are passed to the GPIO_Init() function which sets the GPIO registers

	/* The RX and TX pins are now connected to their AF
	 * so that the USART1 can take over control of the
	 * pins
	 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

	/* Now the USART_InitStruct is used to define the
	 * properties of USART1
	 */
	USART_InitStruct.USART_BaudRate = baudrate;				// the baudrate is set to the value we passed into this init function
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
	USART_InitStruct.USART_StopBits = USART_StopBits_1;		// we want 1 stop bit (standard)
	USART_InitStruct.USART_Parity = USART_Parity_No;		// we don't want a parity bit (standard)
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
	USART_Init(USART1, &USART_InitStruct);					// again all the properties are passed to the USART_Init function which takes care of all the bit setting


	/* Here the USART1 receive interrupt is enabled
	 * and the interrupt controller is configured
	 * to jump to the USART1_IRQHandler() function
	 * if the USART1 receive interrupt occurs
	 */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		 // we want to configure the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART1 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure);							 // the properties are passed to the NVIC_Init function which takes care of the low level stuff

	// finally this enables the complete USART1 peripheral
	USART_Cmd(USART1, ENABLE);
}

void USART1_IRQHandler(void){

	// check if the USART1 receive interrupt flag was set
	if( USART_GetITStatus(USART1, USART_IT_RXNE) ){

		static uint8_t cnt = 0; // this counter is used to determine the string length
		char t = USART1->DR; // the character from the USART1 data register is saved in t

		/* check if the received character is not the LF character (used to determine end of string)
		 * or the if the maximum string length has been been reached
		 */
		if( (t != 'n') && (cnt < MAX_STRLEN) ){
			received_string[cnt] = t;
			cnt++;
		}
		else{ // otherwise reset the character counter and print the received string
			cnt = 0;
			USART_puts(USART1, received_string);
		}
	}
}

/* Initialise USART port x at specified baudrate */
int8_t USART_init(USART_TypeDef* USARTx, uint32_t baudrate)
{
	GPIO_InitTypeDef  GPIO_InitStruct;		/* This is for the GPIO pins used as TX and RX */
	USART_InitTypeDef USART_InitStruct;		/* This is for the USART initilization */
	NVIC_InitTypeDef NVIC_InitStructure;	/* This is used to configure the NVIC
											* (nested vector interrupt controller) */
	GPIO_TypeDef* GPIOx;					/* GPIO port used */

	if(USARTx == USART1) /* USB */
	{

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	/* Enable APB2 peripheral clock (USART1 is on it) */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	/* Same for the gpio pins used */

  		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);	/* DMA1 clock enable */

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;	/* 9 is Tx, 10 is Rx */
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		 /* Configure the USART1 interrupts */
		GPIOx = GPIOA;
		GPIO_PinAFConfig(GPIOx, GPIO_PinSource9, GPIO_AF_USART1);
		GPIO_PinAFConfig(GPIOx, GPIO_PinSource10, GPIO_AF_USART1);

		/* DMA: */
 
		DMA_DeInit(DMA2_Stream7);
  		DMA_StructInit(&DMA_InitStructure);
		DMA_InitStructure.DMA_Channel = DMA_Channel_4;
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; /* Transmit */
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Buffer;
		DMA_InitStructure.DMA_BufferSize = (uint16_t)sizeof(Buffer) - 1;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; /* MAYBE ENABLE */
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; /* MAYBE FUL NOT HALFFULL */
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA2_Stream7, &DMA_InitStructure);
		/* Enable the USART Tx DMA request */
		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		
		/* Enable DMA Stream Transfer Complete interrupt */
		DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
		
		/* Enable the DMA USART1 Tx Stream */
		DMA_Cmd(DMA2_Stream7, ENABLE);
	}
	else
	{
		return -1; /* Incorrect/unhandled USART port */
	}

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;			/* Alternate Function so USART has access */
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			/* Push Pull instead of Open Drain */
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		/* IO speed (NOT baudrate) */
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;			/* Pullup resistors on pins */
	GPIO_Init(GPIOx, &GPIO_InitStruct);

	USART_InitStruct.USART_BaudRate = baudrate;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;/* Data frame size is 8 bits (standard) */
	USART_InitStruct.USART_StopBits = USART_StopBits_1;		/* 1 stop bit (standard) */
	USART_InitStruct.USART_Parity = USART_Parity_No;		/* No parity bit (standard) */
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; /* No flow control (standard) */
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; /* Enable tx and rx */
	USART_Init(USARTx, &USART_InitStruct);

	USART_Cmd(USARTx, ENABLE);

	return 0;
}

void DMA2_Stream7_IRQHandler(void)
{
	/* Test on DMA Stream Transfer Complete interrupt */
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7))
	{
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
	}
}

/*
* USART Interrupt request handler
*/
//void USART1_IRQHandler(void)
//{
//	/* Check if the USART receive interrupt flag was set */
//	if( USART_GetITStatus(USART1, USART_IT_RXNE) )
//	{
//		static uint8_t cnt = 0; /* this counter is used to determine the string length */
//		char t = USART1->DR; /* the character from the USART data register is saved in t */

//		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//		if( ((t != '*') && (cnt < MAX_GPS_STRLEN) && (cnt > 0))
//			|| ((t == '$') && (cnt == 0)) )
//		{
//			if(cnt == 4 && t != 'L')
//			{
//				cnt = 0;
//			}
//			else
//			{
//				received_string[cnt] = t;
//				cnt++;
//			}
//		}
//		// $GPGLL
//		else if( ((t == '*') && (cnt > 0) && ( cnt < (MAX_GPS_STRLEN-1) )))
//		{
//			received_string[cnt] = t;
//			received_string[++cnt] = '\0';
//			memcpy(nmea_data, received_string, cnt+1);
//			//ParseNMEA(nmea_data, cnt+1);
//			// USART_sends(USART1, nmea_data);
//			// USART_sends(USART1, "\r\n");
//			cnt = 0;
//		}
//		else
//		{
//			cnt = 0;
//		}
//	}
//}

/* 
* Send data over USART1 using DMA
*/
int8_t USART1_DMAsends(char *t)
{
	uint16_t len = 0;
	if(*t == 0) return -1;
	while((USART1->SR & USART_FLAG_TC)==0); //wait for non-dma tx completion
	if(!uart1virgin)
		while(DMA_GetFlagStatus(DMA2_Stream7, DMA_FLAG_TCIF7)==0); //wait for dma tx completion
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	while(*t)
	{
		Buffer[len++]= *t++;
		if(len == MAXUSARTBUF-1)
		{
			Buffer[len++] = 0;
			break;
		}
	}
	DMA_InitStructure.DMA_BufferSize = DMA2_Stream7->NDTR = len;
	DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7); //clear transfer complete to allow next one
	DMA_Cmd(DMA2_Stream7, ENABLE);
	uart1virgin=0;

	return 0;
}

/* Send data over specified USART.
* e.g USART_sends(USART1, "Hello World!");
*/
int8_t USART_sends(USART_TypeDef* USARTx, const char *pszString)
{
	int count;
	if( strlen(pszString) > MAX_STRLEN )
		return -1; /* String too long */
	while(*pszString){
		count = 0;
		/* Wait until send register is empty */
		while( !(USARTx->SR & 0x00000040) )
		{
			if (count++ >= timeout)
				return -1; /* Maybe 0, data never sent */
		}			
		USART_SendData(USARTx, *pszString);
		*pszString++;
	}
	return 0;
}
