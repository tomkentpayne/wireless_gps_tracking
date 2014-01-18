/* GPS data parsing code
* maintained by Thomas Payne tp8g10
*/

#include <stm32f4xx.h>
#include <stdio.h>

#include "globals.h"
#include <string.h>
#include <stdlib.h>

//#define MESSAGE_FIX "$GPGLL,5056.27340,N,00123.88000,W,104503.00,A,A*"

double latDeg;
double longDeg;
double GPSreadings[3];

/*
* Parses NMEA GPS strings to obtain lat/long.
* Probably called from a timer and executed before lat/long is logged.
*/
int8_t ParseNMEA(char* nmea_data)
{
	/* Start of GPS message */
	if( (nmea_data[0] == '$') && (nmea_data[1] == 'G') && (nmea_data[2] == 'P') )
	{
		/* Geographic Latitude and Longitude data */
		if( (nmea_data[3] == 'G') && (nmea_data[4] == 'L') && (nmea_data[5] == 'L') )
		{
			double latMins;
			double longMins;
			char tokens[7][16]; /* NMEA data sections, 7 including the * */
			char degrees[4]; /* max 3 digits + \0 */
			char *tmp;
			uint8_t index = 0;
			uint32_t decimalIndex;
			tmp = strtok(&nmea_data[6], ",*"); /* Splits based on delimiter ',' */
			while(tmp != NULL)
			{
				strncpy(tokens[index++], tmp, 16);
				tmp = strtok(NULL, ",*");
			}
			
			if(index < 5)
			{
				return 0; /* NO FIX */
			}

			decimalIndex = strcspn(tokens[0], "."); /* Finds decimal point in token for lat */
			latMins = strtod(tokens[0] + decimalIndex - 2, NULL); /* copies minutes from around the xx'.'xx(x) */
			memcpy(degrees, tokens[0], decimalIndex - 2); /* Copy out Degrees part of token, 2 or 3 bytes */
			degrees[decimalIndex - 2] = '\0'; /* Terminate string */
			latDeg = strtod(degrees, NULL); /* Get degrees */
			if(tokens[1][0] == 'S') /* Maybe use strcomp() instead */
			{
				/* Lat is negative */
				latDeg = -latDeg;
				latMins = -latMins;
			}
			latDeg = latDeg + latMins / 60.0; /* Convert degrees/minutes to degrees */

			decimalIndex = strcspn(tokens[2], "."); /* Finds decimal point in token for lat */
			longMins = strtod(tokens[2] + decimalIndex - 2, NULL); /* copies minutes from around the xx'.'xx(x) */
			memcpy(degrees, tokens[2], decimalIndex - 2); /* Copy out Degrees part of token, 2 or 3 bytes */
			degrees[decimalIndex - 2] = '\0'; /* Terminate string */
			longDeg = strtod(degrees, NULL); /* Get degrees */
			if(tokens[3][0] == 'W') /* Maybe use strcomp() instead */
			{
				/* Lat is negative */
				longDeg = -longDeg;
				longMins = -longMins;
			}
			longDeg = longDeg + longMins / 60.0; /* Convert degrees/minutes to degrees */

			/*
			* ",xx(x)xx.xx(x),N,xx(x)xx.xx(x),W,xxxxxx,A,*"(\0 included)
			* xx(x)|xx.xx(x) is degrees|minutes
			* tokens are everything between ','s
			* Decimal Degrees = Degrees + minutes/60 + seconds/3600
			*/
		}
		else
			return -1; /* Not Lat/Long data */
	}
	else
		return -1; /* Not GPS data */
	return 0;
}

double GetLat(void)
{
	return latDeg;
}

double GetLong(void)
{
	return longDeg;
}
