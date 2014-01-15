/* GPIO code
* maintained by Thomas Payne tp8g10 and Mark Dean
*/

#ifndef __GPIO_H
#define __GPIO_H

void GPIO_init(void);
void GPIO_SetStatusLED(void);
void GPIO_ResetStatusLED(void);
void GPIO_ToggleStatusLED(void);

#endif /* __GPIO_H */
