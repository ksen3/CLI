#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "tty.h"
#include "user.h"


#define MAX_MODULES 10				///< Max modules quantity the CLI can stores
#define MAX_FUNCTIONS_IN_MODULE 10	///< Max functions quantity the module can stores
#define MAX_TOKENS 12				///< Max tokens can be stored


#define S_STORE_IN_FLASH 1			///< Store command history in FLASH
	#ifndef S_STORE_IN_FLASH
	#define S_STORE_IN_RAM 1		///< Store command history in RAM
	#endif
#define S_CMD_SECTOR_SIZE 52 			///< Memory sector size of one command in bytes ! must be a multiple of 4 !
#define S_STORED_NUM_CMDS 10 			///< Command history size
#define S_CURRRENT_CMD_PTR (0x0801F800)	///< Memory field which store number of last command, start of the FLASH page
#define S_START_PAGE_ADDR  (S_CURRRENT_CMD_PTR+4)	///< Address in memory which store start of saved command
#define S_END_PAGE_ADDR    (0x0801FC00)				///< Address in memory which points to the end of page for history

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))	///< Macros calculating size of array

typedef int8_t (*cmd_t)(int32_t argc, const char** argv); ///< Special type for wrapper functions


typedef struct {
    const char *name;
    const cmd_t func;
    const char *help;
    const uint8_t permission;

}function_t;			///< Defines type for function to be stored in module

typedef struct {
    const char *name;
    uint32_t funcNum;
    function_t *funcList;
    const char *help;
}module_t;				///< Defines type for module to be stored in memory

typedef enum {
	// Register errors
	r_error				= -10,	///< Error of module registration in local database
	r_badArguments		= -11,	///< Error connected with arguments
	r_overflow			= -12,	///< Overflow error. Memory for modules is full
	r_alreadyCreated	= -13,	///< Module already exists in local database

	// Parsing errors
	p_argNumber			= -21,	///< Wrong arguments quantity
	p_argType			= -22,	///< Wrong argument type
	p_argValue			= -23,	///< Invalid argument value
	p_argOverflow		= -25,	///< Too many arguments

	// Search errors
	s_moduleNotFound	= -31,	///< Module doesn't exist
	s_functionNotFound	= -32,	///< Function doesn't exist

	// Execute errors
	e_funcExecution		= -41,	///< Function execution error
	e_argValue			= -42,	///< Invalid argument value
	e_argType			= -43,	///< Invalid argument type
	e_argNumber			= -44,	///< Invalid argument amount

	//Other errors
	success				=  0,	///< Success execution
	unknownError 		= -1,	///< Unknown error
}shellStatus_e;					///< Defines possible errors in CLI

//================================================

shellStatus_e s_register(module_t p_module);
shellStatus_e s_parse(char *in_buffer);
shellStatus_e s_find(char *in_token[]);
shellStatus_e s_execute(cmd_t in_function);

shellStatus_e s_storeCmd(const char *in_buffer);
shellStatus_e s_getLastCmd(void);
shellStatus_e s_getCmd(void);

int8_t shell_processing(uint8_t *in_buffer);

#endif // SHELL_H
