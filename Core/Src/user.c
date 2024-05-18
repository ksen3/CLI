#include "user.h"

// Wrapper functions

static int8_t w_login(int32_t argc, char *argv[]);
static int8_t w_logout(int32_t argc, char *argv[]);
static int8_t w_getCurrentUser(int32_t argc, char *argv[]);

//< Set of functions available for CLI in module
static function_t funcList[] = {
    {
        .name = "login",
        .help = "Sign in shell.\r\n" \
        		"Example: 'user login JohnDoe PaSsWoRd'.\r\n" \
        		"Arguments: 1) string | login.\r\n" \
				"           2) string | password.\r\n",
        .func = (cmd_t)w_login,
		.permission = 9,
    },
    {
        .name = "logout",
        .help = "Sign out shell.\r\n" \
        		"Example: 'user logout'.\r\n" \
        		"Arguments: None\r\n",
        .func = (cmd_t)w_logout,
		.permission = 8,
    },
    {
        .name = "info",
        .help = "Get info about current user.\r\n" \
        		"Example: 'user info'.\r\n" \
        		"Arguments: None\r\n",
        .func = (cmd_t)w_getCurrentUser,
		.permission = 9,
    },
};

///< Defines CLI module for work with users
static module_t user_module= {
    .name = "user",
    .funcNum = ARRAY_SIZE(funcList),
    .funcList = funcList,
    .help = "Allow work with users.\r\n"
};

static user_s usersList[MAX_USERS] =
{
		{
				.login = "owner",
				.pass = "owner",
				.permission = p_owner,
		},

		{
				.login = "admin",
				.pass = "admin",
				.permission = p_admin,
		},

		{
				.login = "JohnDoe",
				.pass = "123",
				.permission = p_authorized,
		},

		{
				.login = "default",
				.pass = "default",
				.permission = p_public,
		},
};

user_s currentUser = { "default", "default", p_public };

userStatus_e checkUser(const char* in_user, const char *in_pass)
{
	for(int i=0; i<MAX_USERS; i++)
	{
		if(strcmp(usersList[i].login, "") == 0)	//If NULL login is found than users list if over
		{
			//printf("End of known users list. User wasn't found.\r\n");
			return u_unknownUser;
		}

		if(strcmp(usersList[i].login, in_user) == 0)	//Login checking...
		{
			//printf("User was found. Pass checking...\r\n");
			if(strcmp(usersList[i].pass, in_pass) == 0) //User was found. Pass checking...
			{
				//printf("Pass is OK. Setting permissions...\r\n");
				currentUser = usersList[i];
				return u_ok;
			}

			//printf("Password is wrong. Error.\r\n");
			return u_wrongPass;
		}
	}

	//printf(" wasn't found");
	return u_unknownError;
}

static int8_t w_login(int32_t argc, char *argv[])
{
	if(argc != 2)
		return e_argNumber;

	checkUser(argv[0], argv[1]);
	//strcpy(currentUser.login, argv[0]);
	//strcpy(currentUser.pass, argv[1]);

	return success;
}

static int8_t w_logout(int32_t argc, char *argv[])
{
	if(argc != 0)
		return e_argNumber;

	strcpy(currentUser.login, "default");
	strcpy(currentUser.pass, "default");
	currentUser.permission = 9;

	return success;
}

static int8_t w_getCurrentUser(int32_t argc, char *argv[])
{
	if(argc != 0)
		return e_argNumber;

	char answ[50];
	sprintf(answ, "Current user: %s \r\n" \
				  "Permission level: %d",
				  currentUser.login,
				  currentUser.permission);
	printBuffer(answ);

	return success;
}

int8_t user_init()
{
	s_register(user_module);
	return success;
}

