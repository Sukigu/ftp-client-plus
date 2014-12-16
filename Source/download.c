#include "../Headers/includes.h"
#include "../Headers/user.h"

#define MAX_BUFFER_SIZE 255
#define MAX_SERVER_RESPONSE_NUMBER_SIZE 4

static Settings *userSet;

int main(int argc, char** argv) {

    int sockfd;

    userSet = malloc(sizeof(Settings));
    userSet->username = malloc(255 * sizeof(char)+1);
    userSet->password = malloc(255 * sizeof(char)+1);
    userSet->path = malloc(255 * sizeof(char)+1);
    userSet->domain = malloc(255 * sizeof(char)+1);
    userSet->address = malloc(255 * sizeof(char)+1);

    if(argc != 2){
        printf("\nERROR: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(1);
    }
    
    fillUserSettings(argv[1]); 
    //printf("%s\n%s\n%s\n%s\n%d", userSet->username, userSet->password, userSet->path, userSet->address,userSet->port);
    sockfd = createConnection();
    serverConversation(sockfd);

    close(sockfd);
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

    if ((i = sscanf(url, "ftp://%255[^:]:%255[^@]@%255[^/]/%255[^/]/",username,password,host,path)) != 4 && (i = sscanf(url, "ftp://%255[^/]/%255[^/]/",host,path)) != 2){
        printf("\nERROR: The url you entered is not acceptable.\n");
        exit(2);
    }

    if ((h=gethostbyname(host)) == NULL) {  
            printf("We're sorry but either the server is not responding or this url is invalid.\n");
            exit(3);
        }

    strcpy(userSet->domain,host);

    if(i == 4){
        strcpy(userSet->username,username);
        strcpy(userSet->password,password);
        userSet->port = 21;
    }

    strcpy(userSet->path, path);
    strcpy(userSet->address,inet_ntoa(*((struct in_addr *)h->h_addr)));

    //printf("%s\n%s\n%s\n%s\n%d", userSet->username, userSet->password, userSet->path, userSet->address,userSet->port);
}

/**
 * Creates a conection
 * @param  Description of method's or function's input parameter
 * @return [int] returns File descriptor.
 */
int createConnection(){
    int sockfd;
    struct  sockaddr_in server_addr;

    /*server address handling*/
    bzero((char*)&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(userSet->address);       /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(userSet->port);                  /*server TCP port must be network byte ordered */
    
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
    int response = responseFromServer(sockfd);

    // Login stuff
    if (response == 220) makeLogin(sockfd);
    //else if (response == 225) makeLogin(sockfd); // CHECK
    else {
        printf("Something is wrong with the FTP server. The program will now exit!\n");
        exit(7);
    }
}

void makeLogin(int sockfd){

    if (userSet->username == NULL && userSet->password == NULL)
    {
        printf("The FTP server needs login! We'll trying to get you in anonymous account\n");

        char* login = "user anonymous";
        char* incomplete_pwd = "pass anonymous@";

        size_t pwdSize = strlen(incomplete_pwd) + strlen(userSet->domain) + 1; // '+1' for the null terminator
        char complete[pwdSize];
        strcpy(complete, login);
        strcat(complete, userSet->domain);

        // Send username and wait for response.
        send(sockfd, login, strlen(login),0);
        int response = responseFromServer(sockfd);

        if (response == 331)
        {
            send(sockfd, complete, strlen(complete),0);
            response = responseFromServer(sockfd);

            if (response != 230){
                printf("\nLogin wasn't succefully ended! Sorry.\n");
                exit(9);
            }
        }
        else {
            printf("Sorry. We tried to login but that didn't help. Got your username and password and try again.\n");
            exit(8);
        }
    }else {
        // Send username and wait for response.
        char usernameCompleted[MAX_BUFFER_SIZE];
        strcpy(usernameCompleted,"user ");
        strcat(usernameCompleted,userSet->username);
        strcat(usernameCompleted,"\n");

        send(sockfd, usernameCompleted, strlen(usernameCompleted),0);
        int response = responseFromServer(sockfd);

        if (response == 331)
        {   
            char password_comp[MAX_BUFFER_SIZE];
            strcpy(password_comp,"pass ");
            strcat(password_comp,userSet->password);
            strcat(password_comp,"\n");
            send(sockfd, password_comp, strlen(password_comp),0);
            response = responseFromServer(sockfd);

            if (response != 230){
                printf("Login wasn't succefully ended! Sorry.\n");
                exit(9);
            }
        }
        else {
            printf("Sorry. We tried to login but that didn't help. Got your username and password straight and try again.\n");
            exit(8);
        }
    }
}

/**
 * Return the response of a socket. 
 * @param  File descriptor.
 * @return [Char*] containing the response from server.
 */
int responseFromServer(int sockfd) {
    int receivedFromServer = -1;
    char buffer[255];
    while(receivedFromServer == -1)
        receivedFromServer = recv(sockfd,buffer,255,0);
    buffer[receivedFromServer] = '\0';
            printf("Resposta: %s\n", buffer);
    return atoi(buffer);
}
