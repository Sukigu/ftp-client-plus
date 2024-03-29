#include "../Headers/includes.h"

typedef struct UserSettings {

	// ID information
	char* username;
	char* password;

	// Server Information
	int port;
	char* domain;
	char* address;
	char* path;
	char* newIP;

	// File info.
	int pasv_port;
	unsigned long fileSize;
	char* filename;

} Settings;

static Settings *userSet;

int main(int argc, char** argv) {

    int sockfd;

    userSet = malloc(sizeof(Settings));
    userSet->username = malloc(255 * sizeof(char)+1);
    userSet->password = malloc(255 * sizeof(char)+1);
    userSet->path = malloc(255 * sizeof(char)+1);
    userSet->domain = malloc(255 * sizeof(char)+1);
    userSet->address = malloc(255 * sizeof(char)+1);
    userSet->filename = malloc(255 * sizeof(char)+1);
    userSet->newIP = malloc(255 * sizeof(char)+1);

    if(argc != 2){
        printf("\n> ERROR: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(1);
    }
    else {
    		clearScreen();
    		printf("-------------------------------------------------\n");
    		printf("-            WELCOME TO FTP-CLIENT               \n");
    		printf("-------------------------------------------------\n");
    	}

    fillUserSettings(argv[1]); 
    sockfd = createConnection(21);
    serverConversation(sockfd);
    
    printf("> Now exiting from server.\n");
    char command[strlen("quit\n")];
    strcpy(command,"quit\n");
    send(sockfd, command, strlen(command), 0);

    close(sockfd);
    printf("> Everything went ok. Exit successfully.\n");
    return 0;
}

/**
 * Inserts data into a struct which name is userSet.
 * @param  Receives a url with this template 'ftp://[<user>:<password>@]<host>/<url-path>'
 * @return [VOID]
 */
void fillUserSettings(char* url){

    /*
        TODO: VERIFICAR SIZE EM RFC1738
    */
    struct hostent *h;
    int i;
    char username[MAX_BUFFER_SIZE], password[MAX_BUFFER_SIZE], host[MAX_BUFFER_SIZE],path[MAX_BUFFER_SIZE];

    if ((i = sscanf(url, "ftp://%255[^:]:%255[^@]@%255[^/]/%s",username,password,host,path)) != 4 && (i = sscanf(url, "ftp://%255[^/]/%s",host,path)) != 2){
        printf("\n> ERROR: The url you entered is not acceptable.\n");
        exit(2);
    }

    if ((h=gethostbyname(host)) == NULL) {  
            printf("> We're sorry but either the server is not responding or this url is invalid.\n");
            exit(3);
        }

    strcpy(userSet->domain,host);

    if(i == 4){
        strcpy(userSet->username,username);
        strcpy(userSet->password,password);
    }
    else {
        strcpy(userSet->username,"anonymous");
        strcpy(userSet->password,"12345");
    }
    userSet->port = 21;
    strcpy(userSet->path, path);
    strcpy(userSet->address,inet_ntoa(*((struct in_addr *)h->h_addr)));
    printf(">  INITIAL IP ADDRESS : %s\n",userSet->address);
    printf("> Username : %s\n",userSet->username);
    printf("> Password : %s\n\n",userSet->password);
}

/**
 * Creates a conection
 * @param  Description of method's or function's input parameter
 * @return [int] returns File descriptor.
 */
int createConnection(int port){
    int sockfd;
    struct  sockaddr_in server_addr;


    /*server address handling*/
    bzero((char*)&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(port == 21) server_addr.sin_addr.s_addr = inet_addr(userSet->address);
    else server_addr.sin_addr.s_addr = inet_addr(userSet->newIP);  /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);                  /*server TCP port must be network byte ordered */
    
    /*open an TCP socket*/
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
            perror("socket()");
            exit(4);
        }
    /*connect to the server*/
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
            perror("connect()");
            exit(0);
        }

    return sockfd;
}

/**
 * See if server is 
 * @param  Description of method's or function's input parameter
 * @return [int] returns File descriptor.
 */
void serverConversation(int sockfd){
    // waiting for answer from server and dealing with it
    int response = responseFromServer(sockfd, FALSE);

    // Login stuff
    if (response == 220) makeLogin(sockfd);
    //else if (response == 225) makeLogin(sockfd); // CHECK
    else {
        printf("> Something is wrong with the FTP server. The program will now exit!\n");
        exit(7);
    }
}

/**
 * Try to login in server if needed. If server needs login, and user did not insert data, the function will try a default login.
 * @param  File descriptor.
 * @return [VOID]
 */
void makeLogin(int sockfd){

        // Send username and wait for response.
        char usernameCompleted[MAX_BUFFER_SIZE];
        strcpy(usernameCompleted,"user ");
        strcat(usernameCompleted,userSet->username);
        strcat(usernameCompleted,"\n");

        send(sockfd, usernameCompleted, strlen(usernameCompleted),0);
        int response = responseFromServer(sockfd, FALSE);

        if (response == 331)
        {   
            char password_comp[MAX_BUFFER_SIZE];
            strcpy(password_comp,"pass ");
            strcat(password_comp,userSet->password);
            strcat(password_comp,"\n");
            send(sockfd, password_comp, strlen(password_comp),0);
            response = responseFromServer(sockfd, FALSE);

            if (response != 230){
                printf("> Login wasn't succefully ended! Sorry.\n");
                exit(9);
            }
            else {
                handlingFiles(sockfd);
            }
        }
        else {
            printf("> Sorry. We tried to login but that didn't help. Got your username and password straight and try again.\n");
            exit(8);
        }
}

void handlingFiles(int sockfd){
    send(sockfd,"pasv\n",5,0);
    int answerGivenByServer = parsingURLGivenByServer(sockfd);

    if ( answerGivenByServer == TRUE) {
        int newSockFD = createConnection(userSet->pasv_port);
        char command[MAX_BUFFER_SIZE];
        strcpy(command,"retr ");
        strcat(command,userSet->path);
        strcat(command,"\n");

        send(sockfd,command,strlen(command),0);

        answerGivenByServer = responseFromServer(sockfd,TRUE);

        if (answerGivenByServer == 150)
        {
            get_file_name_from_path();
            FILE* wantedFile;
        	   printf("> STARTING NOW THE TRANSFER OF FILE '%s'.\n\n",userSet->filename);
            wantedFile = fopen(userSet->filename, "wb");
	    char buffer[128];
	    int res = 1; 
		while( (res = read(newSockFD,buffer,1)) > 0){
			fwrite(buffer,1,1,wantedFile);
	}

            fclose(wantedFile);
            close(newSockFD);
        }
        else {
            printf("We're sorry. There was an error trying to access file. Try again.\n");
            exit(10);
        }
    }
}

/**
 * Return TRUE if the parsing is done correctly or exit the program if not.. 
 * @param  File descriptor.
 * @return [INT] if correct.
 */
int parsingURLGivenByServer(int sockfd){
    int receivedFromServer = -1;
    char buffer[MAX_BUFFER_SIZE];
    while(receivedFromServer == -1)
        receivedFromServer = recv(sockfd,buffer,MAX_BUFFER_SIZE,0);
    buffer[receivedFromServer] = '\0';
    printf("> Resposta: %s\n", buffer);

    char temp[255];
    char AUX[2]; AUX[0] = '(';AUX[1] = ')';

    int i = 0, j = 0, CONTROLO = FALSE;
    for (; i < strlen(buffer); i++)
    {
        if ( buffer[i] == AUX[0]){
            temp[j++] = buffer[i];
            CONTROLO = TRUE;
        }
        else if (CONTROLO == TRUE){
            int k = i;
            for (;k < strlen(buffer); k++)
            {
                if (buffer[k] == AUX[1]) {
                    temp[j++] = buffer[k];
                    CONTROLO = END_OF_CYCLE;
                    break;
                }
                temp[j++] = buffer[k];
            }
            temp[k] = '\0';
        }
        else if (CONTROLO == END_OF_CYCLE) break;
    }
    // NO NEED TO CHANGE THE CODE BELOW.

    int parsing;
    // ip_1[3], ip_2[3], ip_3[3], ip_4[3]
    char ip_1[MAX_SERVER_RESPONSE_NUMBER_SIZE], ip_2[MAX_SERVER_RESPONSE_NUMBER_SIZE], ip_3[MAX_SERVER_RESPONSE_NUMBER_SIZE], ip_4[MAX_SERVER_RESPONSE_NUMBER_SIZE], port_1[MAX_SERVER_RESPONSE_NUMBER_SIZE], port_2[MAX_SERVER_RESPONSE_NUMBER_SIZE];
    if ((parsing = sscanf(temp, "(%4[^,],%4[^,],%4[^,],%4[^,],%4[^,],%4[^)]", ip_1, ip_2, ip_3, ip_4, port_1, port_2)) != 6){
        printf("> Server is not responding well. Try again again.\n");
        exit(9);
    }
    else {
        char newIP[MAX_BUFFER_SIZE];
        sprintf (newIP, "%s.%s.%s.%s", ip_1, ip_2, ip_3, ip_4);
        strcpy(userSet->newIP,newIP);
        userSet->pasv_port = 256*atoi(port_1) + atoi(port_2);
        printf(">  NEW IP: '%s:%d'\n",userSet->newIP,userSet->pasv_port);
        return TRUE;
    }
}

/**
 * Fill the variable userSet->filename
 * @param  [NONE]
 * @return [VOID]
 */
void get_file_name_from_path(){
    
    char temp[1]; temp[0] = '/';
    // get last '/' index
    int i = strlen(userSet->path);
    for (; i > 0; i--)
    {
        if( userSet->path[i] == temp[0]){
            i += 1;
            break;
        }
    }
    // get filename and extension.
    char filename[strlen(userSet->path) - i];
    int k=0;
    for(; k < strlen(userSet->path); k++){
        filename[k] = userSet->path[i++];
    }

    strcpy(userSet->filename,filename);
}

/**
 * Fill the variable userSet->fileSize
 * @param  Buffer called message. This buffer contains a message received from server.
 * @return [VOID]
 */
void getFileSizeFromServerResponse(char* message){
    char temp[255];
    char AUX[2]; AUX[0] = '(';AUX[1] = ' ';

    int i = 0, j = 0, CONTROLO = FALSE;
    for (; i < strlen(message); i++)
    {
        if ( message[i] == AUX[0]){
            CONTROLO = TRUE;
        }
        else if (CONTROLO == TRUE){
            int k = i;
            for (;k < strlen(message); k++)
            {
                if (message[k] == AUX[1]) {
                    temp[j++] = message[k];
                    CONTROLO = END_OF_CYCLE;
                    break;
                }
                temp[j++] = message[k];
            }
            temp[k] = '\0';
        }
        else if (CONTROLO == END_OF_CYCLE) break;
    }

    userSet->fileSize = atoi(temp);
}

/**
 * Return the response of a socket. 
 * @param  File descriptor, 'BOOL' with either want to search for size in the message or not.
 * @return [Char*] containing the response from server.
 */
int responseFromServer(int sockfd, int getSize) {
    int receivedFromServer = -1;
    char buffer[MAX_BUFFER_SIZE];
    while(receivedFromServer == -1)
        receivedFromServer = recv(sockfd,buffer,MAX_BUFFER_SIZE,0);
    buffer[receivedFromServer] = '\0';
            printf("> Resposta: %s", buffer);
    if (getSize == TRUE)    getFileSizeFromServerResponse(buffer);

    return atoi(buffer);
}

/**
 * Clears the screen.
 * @param  [NONE]
 * @return [VOID]
 */
void clearScreen(){
	int i = 0;
	for(; i < 50; i++) {printf("\n");}
}
