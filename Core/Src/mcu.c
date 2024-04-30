#include "mcu.h"

static uint32_t hex2dec(const char *hex);
static uint8_t checkMemRegion(uint32_t addr);

// Wrapper functions

static int8_t w_reset(int32_t argc, char *argv[]);
static int8_t w_flashRead(int32_t argc, char *argv[]);
static int8_t w_flashWrite(int32_t argc, char *argv[]);

///< Set of functions available for CLI in module
static function_t funcList[] = {
    {
        .name = "flashRD",
        .help = "Description: Read data from flash memory.\r\n" \
        		"Example: 'mcu flashRD 0x08000000'.\r\n" \
        		"Arguments: 1) address in hex format with prefix (0x02000000).\r\n",
        .func = (cmd_t)w_flashRead,
    },
    {
        .name = "reset",
        .help = "Resets the device. It use without arguments. \r\n" \
        		"Example: 'MCU reset'\r\n" \
        		"Arguments: -\r\n",
        .func = (cmd_t)w_reset,
    },
    {
        .name = "flashWR",
        .help = "Description: Write data to flash memory.\r\n" \
				"Example: 'mcu flashWR 0x0801FC00'.\r\n" \
				"Arguments: 1) address in hex format with prefix '0x' (0x0801FC00). 2) data in hex format with prefix '0x' (0x1234ABCD)\r\n",
        .func = (cmd_t)w_flashWrite,
    },
};

///< Defines CLI module for mcu program module (.h and .c files)
static module_t mcu_module= {
    .name = "MCU",
    .funcNum = ARRAY_SIZE(funcList),
    .funcList = funcList,
    .help = "Contains function for MCU control.\r\n"
};

/**
 * @brief
 * Wrapper function for program reset.
 *
 * @details
 * Wrapper function is designed to call internal function through the CLI.
 * The wrapper function checks argument quantity (shall be = 0).
 * Performs program reset.
 *
 * @param[in] argc
 * Quantity of arguments.
 *
 * @param[in] argv
 * Pointer to arguments representing as strings.
 *
 * @return
 * Execution code (shellStatus_e).
 *
 */
static int8_t w_reset(int32_t argc, char *argv[])
{
	if(argc != 0)
		return e_argNumber;

	mcu_reset();
	return success;
}

/**
 * @brief
 * Wrapper function for MCU's memory reading.
 *
 * @details
 * Wrapper function is designed to call internal function through the CLI.
 * The wrapper function checks arguments quantity and their types.
 * Output data stores in certain address. Memory address shall be defined as argument.
 *
 * @param[in] argc
 * Quantity of arguments.
 *
 * @param[in] argv
 * Pointer to arguments representing as strings.
 *
 * @return
 * Execution code (shellStatus_e).
 *
 */
static int8_t w_flashRead(int32_t argc, char *argv[])
{
	//TODO: refactor cascade if..else
	char outMsg[100];

	if(argc != 1)
		return e_argNumber;


	uint32_t flashAddress = 0, result = 0;

	if((flashAddress = hex2dec(argv[0])) != 0)	//TODO: comparison with zero is wrong. probably...
	{
		result = mcu_memRead(flashAddress);
		sprintf(outMsg, "Address: %x | Data: %x", (unsigned int)flashAddress, (unsigned int)result);
		printBuffer(outMsg);
		return success;
	}
	else
		return e_argValue;
}

/**
 * @brief
 * Wrapper function for MCU's memory writing.
 *
 * @details
 * Wrapper function is designed to call internal function through the CLI.
 * The wrapper function checks argument quantity and their types.
 * Performs writing certain data to certain address of the memory.
 * Memory address and data shall be defined as arguments.
 *
 * @param[in] argc
 * Quantity of arguments.
 *
 * @param[in] argv
 * Pointer to arguments representing as strings.
 *
 * @return
 * Execution code (shellStatus_e).
 *
 */
static int8_t w_flashWrite(int32_t argc, char *argv[])
{

	//TODO: refactor cascade if..else
	char outMsg[100];

	if(argc != 2)
		return e_argNumber;

	uint32_t flashAddress = 0, flashData = 0;

	if((flashAddress = hex2dec(argv[0])) != 0)	//TODO: comparison with zero is wrong. probably...
	{
		if((flashData = hex2dec(argv[1])) != 0)	//TODO: comparison with zero is wrong. probably...
		{
			mcu_memWrite(flashAddress, flashData);

			if(mcu_memRead(flashAddress) == flashData)
			{
				sprintf(outMsg, "Address: %x | Data: %x\r\n", (unsigned int)flashAddress, (unsigned int)flashData);
				printBuffer(outMsg);
				return success;
			}
			else
			{
				sprintf(outMsg, "Wrong data was stored\r\n");
				printBuffer(outMsg);
				return e_funcExecution;
			}
		}
	}
	else
		return e_argValue;

	return unknownError;
}

/**
 * @brief
 * Perform program reset of MCU
 *
 */
void mcu_reset()
{
	SCB->AIRCR = 0x05FA0004;
}

/**
 * @brief
 * Perform MCU's memory reading
 *
 * @param[in] uint32_t address
 * Address to uint32_t data in MCU's memory
 *
 * @return
 * Data stored in certain address
 */
uint32_t mcu_memRead(uint32_t address)
{
	return (*(__IO uint32_t*) address);
}

/**
 * @brief
 * Erases MCU's memory page
 *
 * @param[in] uint32_t address
 * Any address from the page should be deleted
 *
 * @return
 * Execution code
 *
 * @retval 0
 * Erasing if forbidden
 *
 * @retval 1
 * Erasing is allowed
 *
 */
uint8_t mcu_memPageErase(uint32_t address)
{
	//TODO: Return codes, not magic numbers
	if(!checkMemRegion(address))
		return 0;

	HAL_FLASH_Unlock();
	FLASH_PageErase(address);
	CLEAR_BIT (FLASH->CR, (FLASH_CR_PER));
	HAL_FLASH_Lock();

	return 1;
}

/**
 * @brief
 * Write data to the MCU's address
 *
 * @param[in] uint32_t address
 * Memory address for writing procedure
 *
 * @param[in] uint32_t data
 * Data to be stored in memory
 */
void mcu_memWrite(uint32_t address, uint32_t data)
{
	HAL_FLASH_Unlock();
	CLEAR_BIT (FLASH->CR, (FLASH_CR_PER));
	SET_BIT(FLASH->CR, FLASH_CR_PG);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
	CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
	HAL_FLASH_Lock();
}

/**
 * @brief
 * Module registration for shell
 */
void mcu_init()
{
	//TODO return code
	s_register(mcu_module);
}

/**
 * @brief
 * Transforms value from string hex format to numeric dec format
 *
 * @pararm[in] const char *hex
 * Pointer to string representing hex value to be transformed
 *
 * @return
 * Transformed decimal number
 */
static uint32_t hex2dec(const char *hex)
{
	//TODO: Add error codes instead of magic numbers
	//TODO: Think about zero return. Probably it is mistake
	uint32_t decimal = 0, base = 1;
	uint8_t length = 0;

    if(hex[0] != '0' || hex[1] != 'x')
    {
        return 0;
    }
    hex+=2; //delete prefix

    length = strlen(hex);

    for(int i = --length; i >= 0; i--)
    {
        if(hex[i] >= '0' && hex[i] <= '9')
        {
            decimal += (hex[i] - 48) * base;
            base *= 16;
        }
        else if(hex[i] >= 'A' && hex[i] <= 'F')
        {
            decimal += (hex[i] - 55) * base;
            base *= 16;
        }
        else if(hex[i] >= 'a' && hex[i] <= 'f')
        {
            decimal += (hex[i] - 87) * base;
            base *= 16;
        }
        else
        {
        	//TODO shall be removed probably
        	printBuffer("Wrong argument type.\r\n");
        	return 0;
        }

    }
    return decimal;
}

/**
 * @brief
 * Checks if memory region is available to work with CLI
 *
 * @details
 * Limit the memory area for CLI.  For example, the CLI cannot access the first few pages of
 * FLASH memory because the memory for the main program is located there.
 */
static uint8_t checkMemRegion(uint32_t addr)
{
	if(addr < MEM_LOW_EDGE)
		return 0;

	if(addr > MEM_HIGH_EDGE)
		return 0;

	return 1;
}


