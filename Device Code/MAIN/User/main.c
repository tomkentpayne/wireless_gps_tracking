#include <stm32f4xx.h>
#include <stdio.h>
#include <misc.h>
#include <core_cm4.h>

#include "serial.h"
#include "gps.h"
#include "timer.h"
#include "globals.h"
#include "gpio.h"

/*
* README please
* Try to keep setup/peripheral functions etc. in their own files.
* Only functions needed to be public in header files.
* Ditto for variables, but ideally there shouldn't be any global variables exposed raw.
* Commit only .c and .h files to git, no .o, .hex, .uvproj etc. etc.!!!
* functions return 0 by default, -1 if error.
* If a function modifies a variable but can fail, pass a pointer to the variable to change
* 	and still use the return to return error status.
* - Tom
*/
void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}

int main(void)
{
	/* Setup/config things go here:
	* power, timers, serial + main() local vars
	*/

	if( Serial_init() != 0 )
		return -1; /* Serial init failed */
	//USART1_DMAsends("Hel");
	USART1_DMAsends("Hello World!\r\n");
	USART_puts(USART6, "Hello World!USART_puts over USART6\r\n");
	GPIO_init();
	Timer_init();

	GPIO_ResetStatusLED();

	/* Main Loop */
	while(1)
	{
		Delay(2000000L);
		GPIO_ToggleStatusLED();
	}
	return 0;
}
