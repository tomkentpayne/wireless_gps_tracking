/* GPS data parsing code
* maintained by Thomas Payne tp8g10
*/

#ifndef __GPS_H
#define __GPS_H

int8_t ParseNMEA(volatile char* nmea_data, uint8_t len);

#endif /* __GPS_H */
