#include "shell.h"

///< CLI common help string
static const char *commonHelp = "This is common help for CLI." \
						  " There is list of registered modules." \
						  " You can call help for each module.\r\n"	\
						  "Example: mcu help.\r\n";

static const char *unknownModule = "Unknown module name. Try 'help' or '?' to get all modules.\r\n";
static const char *unknownFunction = "Unknown function in module. Try 'help' or '?' to get all functions in module.\r\n";
// static const char *badArguments = "Bad arguments of function. Try 'help' or '?' to get function arguments descriptions.\r\n";

//TODO: refrain from actionPtr pointer, use func instead
static module_t module[MAX_MODULES];	///< Modules database
static char *token[MAX_TOKENS];			///< Pointer to tokens array after command string parsing
static cmd_t actionPtr = NULL;			///< Stores pointer to function to be executed
static function_t *actionFunc;			///< Stores pointer to full function description

static uint8_t countArgc(char *token[]);

/*
 * TODO: Main page
 * Define:
 * - module
 * - token
 * - connection via pytty
 * - wrapper mechanism
 */

/**
 * 	@brief
 * 	Count arguments amount in input buffer.
 *
 * 	@detailed
 * 	It counts all tokens in input message. First token is always module name(or 'help' for CLI).
 * 	Second token is always function name in module(or 'help' for module). The following tokens are arguments list
 * 	for function. Third token may be 'help' for function.
 *
 * 	@param[in] char *token[]
 * 	Tokens list after input buffer parsing.
 *
 * 	@return
 * 	Number of arguments for function
 *
 */
static uint8_t countArgc(char *token[])
{
	int8_t argc = 0;

	while(token[argc])
	    argc++;

	return argc-2; //Due to the two first tokens are module and function names
}


/**
 * 	@brief
 * 	Storing modules in local database (shell register module)
 *
 * 	@param[in]
 * 	module_t in_module
 *
 * 	@return
 * 	Function execution result
 */
shellStatus_e s_register(module_t in_module)
{
    if(in_module.name == NULL)
        return r_badArguments;

    for(int i=0; i<MAX_MODULES; i++)	//Goes through modules database
    {
        if(module[i].name == NULL)		//Available place for module storing was found
        {
            memcpy(&module[i], &in_module, sizeof(module_t));
            return success;
        }

        if(!strcasecmp(module[i].name, in_module.name)) //If the module has already written
            return r_alreadyCreated;
    }
    return r_overflow;	//Database if full
}

/**
 * 	@brief
 * 	Shell command parsing
 *
 * 	@details
 * 	Receives complex string contains command and parse it to simple
 * 	tokens and store it into global "token" array
 *
 * 	@param[in] in_buffer
 * 	Complex string command to parse to tokens
 *
 * 	@return
 *  Function execution result
 */
shellStatus_e s_parse(char *in_buffer)
{
       char *lexeme = NULL;
       uint8_t cnt = 0;

       lexeme = strtok(in_buffer, " ");   //First lexeme (module name)
       token[cnt] = lexeme;
       cnt++;

       while (lexeme != NULL)
       {
          lexeme = strtok (NULL, " ");
          token[cnt] = lexeme;
          cnt++;

          if(cnt >= MAX_TOKENS)
              return p_argOverflow; // Arguments overflow
       }
       token[--cnt] = 0;    //Delete null argument (last position)

       return success;
}

/**
 * 	@brief
 * 	Shell token processing
 *
 * 	@details
 * 	This function tries to find the requested action by parsing tokens list.
 * 	Function receives pointer to token array as input argument, then applies following algorithm:
 * 	1. Tries to find the requested module.
 * 	2. Tries to find the requested function.
 * 	3. Chooses action to be executed.
 * 	4. Stores this action to actionPtr
 * 	Also this function can output hint for module or function if it was requested
 *
 * 	@param[in] char* token[]
 * 	Array of separated tokens. One token is one string.
 *
 * 	@return
 * 	Function execution result
 */
shellStatus_e s_find(char *token[])
{
	//Common help
	if(strcasecmp("help", token[0]) == 0 || strcasecmp("?", token[0]) == 0 )
	{
		printBuffer(commonHelp);

		for(int i=0; i<MAX_MODULES; i++) //Goes through modules database
		{
			if(module[i].name == 0)	//If modules are over
				break;				//exit from the cycle

			printBuffer(module[i].name); // Output all available modules
			printBuffer("\r\n");
		}
		return success;
	} // end common help

	//Module search (first token is always module name)
    for(int i=0; i<MAX_MODULES; i++)	//cycle through modules
    {
        if(strcasecmp(module[i].name, token[0]) == 0)	//module was found
        {
            if(strcasecmp(token[1], "help" ) == 0 || strcasecmp("?", token[1]) == 0 ) //module's help was requested
            {
            	actionPtr = NULL;

            	function_t *funcInModule = module[i].funcList; //Get all functions in module
            	printBuffer(module[i].help);
            	printBuffer("Following functions are declared in this module: \r\n");
            	for(int funcNum=0; funcNum<module[i].funcNum; funcNum++)	// Output all available functions in module
            	{
            		printBuffer(module[i].funcList[funcNum].name);
            		printBuffer("\r\n");
            	}
            	return success;
            }

            //Function search (second token is always function name)
            for(uint32_t j=0; j<module[i].funcNum; j++)	//cycle through functions in module
            {
                if(strcasecmp(module[i].funcList[j].name, token[1]) == 0)	//function was found
                {
                    if(strcasecmp(token[2], "help") == 0 || strcasecmp("?", token[2]) == 0 ) //function's help was requested
                    {
                    	actionPtr = NULL;
                    	printBuffer(module[i].funcList[j].help);
                    	return success;
                    }
                    else	//function's action was requested
                    {
                    	actionPtr = module[i].funcList[j].func;	//Save pointer on the found function
                    	actionFunc = &(module[i].funcList[j]);
                    	return success;
                    }
                }
            }
            printBuffer(unknownFunction);	//If requested function was not found
            return s_functionNotFound;
        }
    }
    printBuffer(unknownModule);	//If requested module was not found
    return s_moduleNotFound;
}

/**
 * @brief
 * Executes requested function
 *
 * @details
 * Calles function received as input argument
 *
 * @param[in] cmd_t function
 * Represents wrapper function to be executed
 *
 * @return
 * shellStatus_e execution code
 *
 */
shellStatus_e s_execute(cmd_t function)
{
	int8_t execResult = unknownError;	//Stores wrapper function execution result

	if(function == NULL)
		return e_funcExecution;

	uint8_t argc = 0;
	argc = countArgc(token);

	execResult = function(argc, &token[2]);	//Execute function

	//Output message with function execution info. Message depends on execution code.
	switch(execResult)
	{
		case success:
		{
			break;
		}
		case unknownError:
		{
			printBuffer("Unknown error\r\n");
			break;
		}
		case e_funcExecution:
		{
			printBuffer("Execution error or function doesn't create.\r\n");
			break;
		}
		case e_argValue:
		{
			printBuffer("Invalid argument value.\r\n");
			break;
		}
		case e_argType:
		{
			printBuffer("Invalid argument type.\r\n");
			break;
		}
		case e_argNumber:
		{
			printBuffer("Invalid arguments amount.\r\n\r\n");
			break;
		}
		default:
		{
			printBuffer("Unknown error\r\n");
			break;
		}
	}

	return success;
}

/**
 * @brief
 * Store input command (tty_rx_buffer) in FLASH or RAM
 *
 * @details
 * This function is intended for storing history of input commands in MCU's internal memory.
 * Storage is implemented as cyclic buffer. It has maximum value represented as S_STORED_NUM_CMDS
 * macro definition. If history reach maximum value, the oldest command is rewrites. This cyclic
 * buffer can be considered as array with indexes from 0 to S_STORED_NUM_CMDS.
 *
 * FLASH OR RAM
 * ------------
 * It has two options:
 * 	1. Keep a history in FLASH. To select this option uncomment S_STORE_IN_FLASH macro definition.
 * 		If this option is chosen CLI keep a history in FLASH memory. The whole FLASH page
 * 		is allocated for this purpose. It starts from S_START_PAGE_ADDR and ends with
 * 		S_END_PAGE_ADDR. By default this is the second from the bottom page.
 * 	2. Keep a history in RAM. To select this option comment S_STORE_IN_FLASH macro definition.
 * 		If this option is chosen CLI keep a history in RAM. For this purpose the RAM memory
 * 		is allocated statically as array.
 *
 * MEMORY SPILTTING
 * ----------------
 * Memory space consists from 256 uint32_t words (or 1024 bytes). It is whole page for medium-density
 * STM32F103 MCU. This space is divided to following fields:
 * 	1. First 4 bytes contains  numeric value. This value is points to the last command is cyclic buffer.
 * 	2. Than storage contains S_STORED_NUM_CMDS equal parts which called _sector_.
 * 	_Sector_ is part of storage which can store input command. The size of the sector defines
 * 	S_CMD_SECTOR_SIZE macro definition.
 *
 * 	By default in can be represented by following schema:

 * 	+------------------------+----------------+----------------+-----+----------------+
 * 	|      0x0801F800        |   0x0801F804   |   0x0801F838   | ... |   0x0801F9D8   |
 *  +------------------------+----------------+----------------+-----+----------------+
 * 	|        4 bytes         |     52 bytes   |    52 bytes    | ... |    52 bytes    |
 * 	+------------------------+----------------+----------------+-----+----------------+
 *	| Number of last command | Stored command | Stored command | ... | Stored command |
 *	+------------------------+----------------+----------------+-----+----------------+
 *
 *	@param[in] const char *in_buffer
 *	Pointer to input buffer. This string will be stored in history
 *
 * @return
 * shellStatus_e Function execution result
 *
 * @todo: make option to store available commands only
 */
shellStatus_e s_storeCmd(const char *in_buffer)
{
#ifdef S_STORE_IN_FLASH

	uint32_t temp[256] = {0,};	///< Temporary buffer intended to work with info from flash. Represents FLASH page.
	uint32_t offset = 0;		///< Points to necessary command if temporary buffer

	// Copy from FLASH to RAM (to temp array)
	temp[0] = (*(__IO uint32_t*)(S_CURRRENT_CMD_PTR));
	for(int i=0; i<(S_END_PAGE_ADDR-S_START_PAGE_ADDR)/4; i++)	//255 uint32_t variables are in flash page
		temp[i+1] = (*(__IO uint32_t*)(S_START_PAGE_ADDR+i*4));

	// Increment pointer to last stored cli command in RAM
	temp[0]++;
	if(temp[0] >= S_STORED_NUM_CMDS) temp[0] = 0;

	// Store last cli command in RAM (in temp array)
	offset = (temp[0] * (S_CMD_SECTOR_SIZE/4)) + 1;
	memcpy(&(temp[offset]), in_buffer, S_CMD_SECTOR_SIZE);

	// Erase FLASH page and save RAM in FLASH
	HAL_FLASH_Unlock();

//	FLASH_PageErase(S_CURRRENT_CMD_PTR);
//	CLEAR_BIT (FLASH->CR, (FLASH_CR_PER));

	FLASH_PageErase(S_CURRRENT_CMD_PTR);
	CLEAR_BIT (FLASH->CR, (FLASH_CR_PER));

	SET_BIT(FLASH->CR, FLASH_CR_PG);
	for(int i=0; i<256; i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, S_CURRRENT_CMD_PTR+i*4, temp[i]);

	CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

	HAL_FLASH_Lock();

	return success;
#elif defined(S_STORE_IN_RAM)
	static uint8_t cmdBuffer[1024] = {0, };
	static uint8_t lastCmd = 0;

	// Store last cli command in RAM
	uint8_t offset = (lastCmd * S_CMD_SECTOR_SIZE);
	memcpy(&(cmdBuffer[offset]), in_buffer, S_CMD_SECTOR_SIZE);

	lastCmd++;
	if(lastCmd >= S_STORED_NUM_CMDS) lastCmd = 0;

	return success;
#endif
	return e_funcExecution;
}

/**
 * @brief
 * Print on the screen last input command
 *
 * @return
 * shellStatus_e Function execution result
 *
 * @note: This function works with bytes(uint8_t pointer). S_CMD_SECTOR_SIZE mustn't be divided by 4
 *
 */
shellStatus_e s_getLastCmd(void)
{
	extern uint8_t tty_rx_buffer[128];
	extern uint8_t *cmd_string_ptr;
	extern int8_t tty_rx_buffer_size;

	uint32_t lastCmd = 0xffffffff;
	uint8_t *ptr = NULL;
	lastCmd = (*(__IO uint32_t*)(S_CURRRENT_CMD_PTR)); //get number of last command in cyclic buffer

	if(!(lastCmd >=0 && lastCmd <= 9))
		return e_funcExecution;

	ptr = S_START_PAGE_ADDR + (lastCmd*S_CMD_SECTOR_SIZE); //get address of last command in memory

	if(ptr == NULL)
		return e_funcExecution;

	// Output result
	printBuffer(ptr);
	strcpy(tty_rx_buffer, ptr);
	cmd_string_ptr = tty_rx_buffer+strlen(ptr)-1;
	tty_rx_buffer_size = strlen(ptr);
	return success;
}

/**
 * @brief
 * Print on the screen selected input command
 *
 * @details
 * Selects command from history. Depth of search increases every function call.
 * This function designed to search certain command. It increase depth value with
 * every call. Imply that every call of this function appropriate every up arrow button press.
 *
 * @return
 * shellStatus_e Function execution result
 *
 * @note: This function works with bytes(uint8_t pointer). S_CMD_SECTOR_SIZE mustn't be divided by 4
 *
 * @todo: Add down arrow button press processing
 */
shellStatus_e s_getCmd(void)
{
	extern uint8_t tty_rx_buffer[128];
	extern uint8_t *cmd_string_ptr;
	extern int8_t tty_rx_buffer_size;

	static int8_t depth = 0;
	static uint32_t prevCurrent = 0xffffffff;

	uint32_t lastCmd = 0xffffffff;
	uint8_t *ptr = NULL;

	if((*(__IO uint32_t*)(S_CURRRENT_CMD_PTR)) == 0xFFFFFFFF)
		return e_funcExecution;

	if(prevCurrent != (*(__IO uint32_t*)(S_CURRRENT_CMD_PTR)))
		depth = 0;

	lastCmd = (uint8_t)(*(__IO uint32_t*)(S_CURRRENT_CMD_PTR)) - depth;

	if((int8_t)lastCmd <= (int8_t)-1)
	{
		depth = (*(__IO uint32_t*)(S_CURRRENT_CMD_PTR)) - S_STORED_NUM_CMDS + 1;
		lastCmd = S_STORED_NUM_CMDS - 1;
	}

	ptr = S_START_PAGE_ADDR + (lastCmd*S_CMD_SECTOR_SIZE);

	depth++;
	if(depth >= S_STORED_NUM_CMDS)
		depth = 0;

	if(ptr == NULL)
		return e_funcExecution;

	if(*(uint32_t *)ptr == 0xFFFFFFFF)
		return e_funcExecution;

	printBuffer(ptr);
	strcpy(tty_rx_buffer, ptr);
	cmd_string_ptr = tty_rx_buffer+strlen(ptr)-1;
	tty_rx_buffer_size = strlen(ptr);

	prevCurrent = (*(__IO uint32_t*)(S_CURRRENT_CMD_PTR));

	return success;
}

/**
 * @brief
 * Contains functions which describe main shell algorithm
 *
 * @param[in] uint8_t *in_buffer
 * Input string received via UART
 *
 * @return
 * Always 0. It's wrong
 *
 * @todo:
 * 		  - status variable is redundant
 * 		  - return always 0
 */
int8_t shell_processing(uint8_t *in_buffer)
{
	int8_t status = -1;

	//todo: store valid only commands
	status = s_storeCmd(in_buffer);

	status = s_parse(in_buffer);
	status = s_find(token);

	//Checking current user and permissions
#ifdef INC_USER_H_
	extern user_s currentUser;

	//printBuffer(actionFunc->name);

	if(strcmp(token[0], "user") != 0)
	{
		status = checkUser(currentUser.login, currentUser.pass);

		if(status != u_ok)	//Does user exist in list?
		{
			printBuffer("Unknown user. Execution forbidden");
			return -1;
		}

	}

	if(actionFunc->permission < currentUser.permission)
	{
		printBuffer("User authorized. Execution forbidden");
		return -1;
	}
#endif

	status = s_execute(actionPtr);

//	switch(status)
//	{
//		case success:
//		{
//			printBuffer("Success!\r\n");
//			break;
//		}
//		case e_argNumber:
//		{
//			printBuffer(badArguments);
//			break;
//		}
//		case e_argValue:
//		{
//			printBuffer("Arg error\r\n");
//			break;
//		}
//	}

	memset(token, 0, sizeof(token));
	actionPtr = NULL;

	return 0;
}

