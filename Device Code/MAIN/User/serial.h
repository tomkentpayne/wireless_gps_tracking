/* USART serial code
* used by GPS, GPRS comms
* maintained by Thomas Payne tp8g10
*/

#ifndef __SERIAL_H
#define __SERIAL_H

int8_t Serial_init(void);
int8_t USART_sends(USART_TypeDef* USARTx, const char *pszString);
int8_t USART1_DMAsends(char *t);

#endif /* __SERIAL_H */
