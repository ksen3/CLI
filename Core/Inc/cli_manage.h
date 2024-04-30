#ifndef INC_CLI_MANAGE_H_
#define INC_CLI_MANAGE_H_

#include "shell.h"


typedef struct {
	uint8_t major;
	uint8_t minor;
}cliVersion_t;		///< Define type for CLI info


int8_t cliManager_init();


#endif /* INC_CLI_MANAGE_H_ */
