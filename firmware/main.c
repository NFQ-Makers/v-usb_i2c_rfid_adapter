/* Name: main.c
 * Project: hid-mouse, a very simple HID example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-07
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.

We use VID/PID 0x046D/0xC00E which is taken from a Logitech mouse. Don't
publish any hardware using these IDs! This is for demonstration only!
*/

#include <stdlib.h>         /* for utoa() */
#include <string.h>         /* for strlen() */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

PROGMEM const char usbHidReportDescriptor[27] = { /* USB report descriptor, size must match usbconfig.h */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x07,                    // USAGE (Keypad)
    0x95, 0x00,                    // REPORT_COUNT (0)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Key Codes)???
    0x09, 0x00,                    //   USAGE (0)
    0x19, 0x53,                    //   USAGE_MINIMUM (53h)
    0x29, 0x63,                    //   USAGE_MAXIMUM (63h)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x11,                    //   LOGICAL_MAXIMUM (17)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x00,                    //   INPUT (Data,Array)
    0xC0,                          // END COLLECTION
};


static uchar reportBuffer;
static uchar idleRate;   /* repeat rate for keyboards, never used for mice */
static char  message[12] = {0};

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (void *)data;

    /* The following requests are never used. But since they are required by
     * the specification, we implement them in this example.
     */
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            usbMsgPtr = (void *)&reportBuffer;
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

int addCardNum(uint32_t num) {
	if (message[0]==0 && reportBuffer==0) {
		ultoa(num, message, 10);
		int len = strlen(message);
		message[len] = '\n';
		message[len+1] = '\0';
		return 0;
	}
	return -1;
}

/**
 * ASCII to KEY CODE mapper
 * @param c character to convert to key code
 * @return key code to send via USB
 */
uint8_t charToKey(char c) {
	if (c==47) return 2;  // /
	if (c==42) return 3;  // *
	if (c==45) return 4;  // -
	if (c==43) return 5;  // +
	if (c==10) return 6;  // \n
	if (c>=49 && c<=57) { // 1-9
		return c-49+7;
	}
	if (c==48) return 16; // 0
	if (c==46) return 17; // .
	return 0;
}

char arrayShift(char* arr) {
	char result = arr[0];
	int len = strlen(arr);
	int i;
	for (i=0; i<len-1; i++) {   
		arr[i]=arr[i+1];
	}
	arr[len-1] = 0;
	return result;
}
/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void)
{
	uchar   i;

	DDRC = 0b00000000;

    wdt_enable(WDTO_1S);
    /* If you don't use the watchdog, replace the call above with a wdt_disable().
     * On newer devices, the status of the watchdog (on/off, period) is PRESERVED
     * OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    for(;;){                /* main event loop */
		
		// check key presses
		if (PINC&2 == 2) {
			addCardNum(1234567890UL);
		}
		
        wdt_reset();
        usbPoll();
        if(usbInterruptIsReady()){
            /* called after every poll of the interrupt endpoint */
			reportBuffer=charToKey(arrayShift(message));
            usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
        }
    }
}

/* ------------------------------------------------------------------------- */
