/* GPS data parsing code
* maintained by Thomas Payne tp8g10
*/

#ifndef __GPS_H
#define __GPS_H

int8_t ParseNMEA(/*char* nmea_data*/ void);
double GetLat(void);
double GetLong(void);

#endif /* __GPS_H */
