/*
	Header file containing general C headers.
	Header file containing all declarations of functions
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

// declarations

void fillUserSettings(char* );
int createConnection();
void serverConversation(int );
void makeLogin(int );
int responseFromServer(int );