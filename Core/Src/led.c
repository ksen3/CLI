#include "led.h"

static int8_t w_ledSwitch(int32_t argc, char *argv[]);

///< Set of functions available for CLI in module
static function_t funcList[] = {
    {
        .name = "switch",
        .help = "Description: This function set high or low level on pin.\r\n" \
        		"Example: 'led switch 2 1' - turn on second led; 'led switch 4 0' - turn off fouth led.\r\n" \
				"Arguments: 1) led number: led number from 1 to 4, 2)led state: 1 - ON, 0 - OFF.\r\n",
        .func = (cmd_t)w_ledSwitch,
    },

};

///< Defines CLI module for led program module (.h and .c files)
static module_t led_module= {
    .name = "led",
    .funcNum = ARRAY_SIZE(funcList),
    .funcList = funcList,
    .help = "LED module is created for manage diodes.\r\n"
};

/**
 * @brief
 * Wrapper function for LED control.
 *
 * @details
 * Wrapper function is designed to call internal function through the CLI.
 * The wrapper checks the arguments number, their types and limit their
 * possible values. In this case wrapper refers to the ledSwitch function
 * created for LED control.
 *
 * @param[in] argc
 * Quantity of arguments.
 *
 * @param[in] argv
 * Pointer to arguments representing as strings/
 *
 * @return
 * Execution code (shellStatus_e).
 */
static int8_t w_ledSwitch(int32_t argc, char *argv[])
{
	ledNum_e L = unknownState;
	ledState_e S = unknownLed;

	if(argc != 2)
		return e_argNumber;

	L = atoi(argv[0]);
	if(L < 1 || L > 4)
		return e_argValue;

	S = atoi(argv[1]);
	if(S != 0 && S!=1)
		return e_argValue;

	ledSwitch(L, S);
    return 0;
}


/**
 * @brief
 * Module registration for shell
 *
 * @return
 * Execution code (shellStatus_e)
 */
int8_t led_init()
{
	s_register(led_module);
	return success;
}

/**
 * @brief
 * Sets certain LED state
 *
 * @param[in] ledNu,_e num
 * LED number (from 1 to 4)
 *
 * @param[in] ledState_e state
 * LED state (on, off)
 *
 * @return
 * In every case 1
 */
int32_t ledSwitch(ledNum_e num, ledState_e state)
{
	switch(num)
	{
		case led1:
		{
			if(off == state)
				GPIOB->ODR &= ~(1<<1);
			else if(on == state)
				GPIOB->ODR |= (1<<1);
			else
				return 1;

			break;
		}

		case led2:
		{
			if(off == state)
				GPIOB->ODR &= ~(1<<3);
			else if(on == state)
				GPIOB->ODR |= (1<<3);
			else
				return 1;

			break;
		}

		case led3:
		{
			if(off == state)
				GPIOA->ODR &= ~(1<<9);
			else if(on == state)
				GPIOA->ODR |= (1<<9);
			else
				return 1;

			break;
		}

		case led4:
		{
			if(off == state)
				GPIOA->ODR &= ~(1<<10);
			else if(on == state)
				GPIOA->ODR |= (1<<10);
			else
				return 1;

			break;
		}

		default:
		{
			return 1;
			break;
		}
	}		/* End switch */

	return 1;
}
