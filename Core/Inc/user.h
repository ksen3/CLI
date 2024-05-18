#ifndef INC_USER_H_
#define INC_USER_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "shell.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))	///< Macros calculating size of array
#define MAX_USERS 10								///< Stricts max number of users
#define MAX_LOGIN_SYMBOLS 20
#define MAX_PASS_SYMBOLS 20

//char *user = NULL;
//char *password = NULL;

typedef enum{
	p_owner = 0,
	p_admin = 1,
	//...TBD
	p_authorized = 8,
	p_public = 9,
}userRights_e;

typedef enum{
	u_ok = 0,
	u_unknownUser = 1,
	u_noPermissisons = 2,
	u_usersOverflow = 3,
	u_unknownError = 4,
	u_wrongPass = 5,
}userStatus_e;

typedef struct{
	char login[MAX_LOGIN_SYMBOLS];
	char pass[MAX_PASS_SYMBOLS];
	userRights_e permission;
}user_s;

int8_t user_init();
userStatus_e checkUser(const char* in_user, const char *in_pass);

#endif /* INC_USER_H_ */
