/* USART serial code
* used by GPS, GPRS comms
* maintained by Thomas Payne tp8g10
*/

#ifndef __SERIAL_H
#define __SERIAL_H

int8_t Serial_init(void);
int8_t USART_sends(USART_TypeDef* USARTx, char *pszString);
int8_t USART1_DMAsends(char *t);
int8_t USART6_DMAsends(char *t);
void USART_puts(USART_TypeDef* USARTx, volatile char *s);
char* NMEA_data(void);

#endif /* __SERIAL_H */
