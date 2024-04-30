#ifndef INC_TTY_H_
#define INC_TTY_H_

#include "stm32f1xx_hal.h"

//#define TTY_ECHO_MODE 0	///< Copy input to console

void dma_init(void);
void waitChar(void);
void printChar(void);

int8_t printBuffer(const char *buffer);

#endif /* INC_TTY_H_ */
