#include "../Headers/includes.h"
#include "../Headers/user.h"

static Settings *userSet;

int main(int argc, char** argv) {

    userSet = malloc(sizeof(Settings));
    if(argc != 2){
        printf("\nERROR: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(1);
    }
    
    fillUserSettings(argv[1]);

    return 0;
}

void fillUserSettings(char* url){

    struct hostent *h;
    int i;
    char username[255], password[255], host[255],path[255];

    if ((i = sscanf(url, "ftp://%255[^:]:%255[^@]@%255[^/]/%255[^/]/",username,password,host,path)) != 4 && (i = sscanf(url, "ftp://%255[^/]/%255[^/]/",host,path)) != 2){
        printf("\nERROR: The url you entered is not acceptable.\n");
        exit(2);
    }

    if ((h=gethostbyname(host)) == NULL) {  
            herror("gethostbyname");
            exit(1);
        }

    // INITIATE VALUES 
    userSet->username = NULL;
    userSet->password = NULL;

    if(i == 4){
        userSet->username = username;
        userSet->password = password;
    }

    userSet->path = path;
    userSet->address = inet_ntoa(*((struct in_addr *)h->h_addr));

    printf("%s\n%s\n%s\n%s\n", userSet->username, userSet->password, userSet->path, userSet->address);
}
