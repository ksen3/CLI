#include "cli_manage.h"

cliVersion_t ver = {0, 1};	///< Defines CLI information


static int8_t w_getCliVersion(int32_t argc, char *argv[]);

///< Set of functions available for CLI in module
static function_t funcList[] = {
	{
		.name="version",
		.help = "Get CLI version.\r\n",
		.func = (cmd_t)w_getCliVersion,
	},
};

///< Defines CLI module for cli_manager program module (.h and .c files)
static module_t cli_module = {
	.name = "cli",
	.funcNum =  ARRAY_SIZE(funcList),
	.funcList = funcList,
	.help = "CLI manager module. In this module you can acsess the managment console.\r\n"
};

/**
 * @brief
 * Module registration for shell
 *
 * @return
 * Execution code (shellStatus_e)
 */
int8_t cliManager_init()
{
	s_register(cli_module);
	return success;
}

/**
 * @brief
 * Wrapper function for CLI version output
 *
 * @details
 * Wrapper function is designed to call internal function through the CLI. In this case the wrapper
 * refers to the internal variable representing the CLI information.
 *
 * @param[in] argc
 * Quantity of arguments
 *
 * @param[in] argv
 * Pointer to arguments representing as strings
 *
 * @return
 * Execution code (shellStatus_e)
 */
static int8_t w_getCliVersion(int32_t argc, char *argv[])
{
	char msg[100];

	if(argc != 0)
		return e_argNumber;

	sprintf(msg, "Version %d.%d", ver.major, ver.minor);
	return printBuffer(msg);
}
