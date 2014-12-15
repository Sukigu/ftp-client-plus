#include <stdio.h>

int main(int argc, char** argv) {

	if(argv != 2){
		printf("ERROR: ./download ftp://[<user>:<password>@]<host>/<url-path>\n");
	}

	return 0;
}
