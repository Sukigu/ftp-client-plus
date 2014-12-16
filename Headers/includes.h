/*
	Header file containing general C headers.
	Header file containing all declarations of functions
*/

// DEFINES
#define MAX_BUFFER_SIZE 256
#define MAX_SERVER_RESPONSE_NUMBER_SIZE 4
#define TRUE 0
#define FALSE 1
#define END_OF_CYCLE 3


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
void handlingFiles(int );
int parsingURLGivenByServer(int );
int responseFromServer(int );