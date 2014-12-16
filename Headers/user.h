#include "../Headers/includes.h"


/**
 * Struct to store some user information to be avaiable easily.
 */
typedef struct UserSettings {

	// ID information
	char* username;
	char* password;

	// Server Information
	int port;
	char* domain;
	char* address;

	// File info.
	char* path;

} Settings;
