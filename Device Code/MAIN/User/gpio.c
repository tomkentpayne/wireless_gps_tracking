/* GPIO code
* maintained by Thomas Payne tp8g10 and Mark Dean
*/

#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>

static uint32_t LEDstatus;

/*
* Initialise GPIO for status LED
*/
void GPIO_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	LEDstatus = 0;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14; // we want to configure all LED GPIO pins
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT; 		// we want the pins to be an output
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 	// this sets the GPIO modules clock speed
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; 	// this sets the pin type to push / pull (as opposed to open drain)
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 	// this sets the pullup / pulldown resistors to be inactive
	GPIO_Init(GPIOC, &GPIO_InitStruct); 			// this finally passes all the values to the GPIO_Init function which takes care of setting the corresponding bits.

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
}

/* Set LED on */
void GPIO_SetStatusLED(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_14);
	LEDstatus = 1;
}

/* Set LED off */
void GPIO_ResetStatusLED(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_14);
	LEDstatus = 0;
}

/* Toggle LED on/off */
void GPIO_ToggleStatusLED(void)
{
	if(LEDstatus)
		GPIO_ResetBits(GPIOC, GPIO_Pin_14);
	else
		GPIO_SetBits(GPIOC, GPIO_Pin_14);
	LEDstatus = 1 - LEDstatus;
}

