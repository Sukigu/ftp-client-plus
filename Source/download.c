#include "../Headers/includes.h"
#include "../Headers/user.h"

static Settings userSet;

int main(int argc, char** argv) {

	if(argc != 2){
		printf("\nERROR: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
		exit(1);
	}
	
	fillUserSettings(argv[1]);

	return 0;
}

void fillUserSettings(char* url){
	if ((h=gethostbyname(url)) == NULL) {  
            herror("gethostbyname");
            exit(1);
        }

        printf("Host name  : %s\n", h->h_name);
        printf("Address Type  : %d\n", h->h_addrtype);
        printf("Length : %d\n", h->h_length);
        printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));
	printf("%s\n", url);
}
