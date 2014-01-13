#include <stm32f4xx.h>
#include <stdio.h>
#include <misc.h>
#include <core_cm4.h>

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

int main(void)
{
	/* Setup/config things go here:
	* power, timers, serial + main() local vars
	*/
	
	/* Main Loop */
	while(1)
	{
		
	}
	return 0;
}
