#ifndef INC_LED_H_
#define INC_LED_H_

#include "stm32f1xx_hal.h"
#include "stdlib.h"

#include "shell.h"


typedef enum {
	off = 0,
	on = 1,
	unknownState = 255,
}ledState_e;	///< Possible led states

typedef enum {
	led1 = 1,
	led2 = 2,
	led3 = 3,
	led4 = 4,
	unknownLed = 255,
}ledNum_e;	///< Possible numbers of leds


int8_t led_init();
int32_t ledSwitch(ledNum_e num, ledState_e state);



#endif /* INC_LED_H_ */
