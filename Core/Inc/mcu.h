#ifndef INC_MCU_H_
#define INC_MCU_H_

#include "stm32f1xx_hal.h"
#include "stdlib.h"
#include "shell.h"

#define MEM_LOW_EDGE	0x0800C000		///< Lower bound of available FLASH memory for CLI
#define MEM_HIGH_EDGE	0x0801FC00		///< Upper bound of available FLASH memory for CLI


void mcu_reset();
void mcu_FW();
void mcu_SW();
uint8_t mcu_memPageErase(uint32_t address);
uint32_t mcu_memRead();
void mcu_memWrite();

void mcu_init();



#endif /* INC_MCU_H_ */
