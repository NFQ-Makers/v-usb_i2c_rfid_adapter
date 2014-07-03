#ifndef GLOBAL_H_
#define GLOBAL_H_

//Target device I2C address
#define LOCAL_ADDR	100 << 1

#include <stdlib.h>         /* for ultoa() */
#include <string.h>         /* for strlen() */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <util/twi.h>       /* for I2C(TWI) comunications */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"

#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/parity.h>

#include "i2c/avrlibdefs.h"
#include "i2c/avrlibtypes.h"
#include "i2c/i2c.h"

#endif /* GLOBAL_H_ */
