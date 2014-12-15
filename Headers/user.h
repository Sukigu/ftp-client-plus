#include "../Headers/includes.h"

typedef struct UserSettings {

	// ID information
	char* username;
	char* password;

	// Server Information
	int port;
	char** address;
	
	// File info.
	char** path;

} Settings;



