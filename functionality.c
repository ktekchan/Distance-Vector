/*-------------------------------------------------

Name: Khushboo Tekchandani
UBIT name: ktekchan

This file contains the functions that implement the
the funtionality of the program

--------------------------------------------------*/
#include "logger.h"
#include "PA3_headers.h"


void academic_integrity(){
	printf("I have read and understood the course academic integrity policy\
		located at\
		http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/\
		index.html#integrity\n");
	cse4589_print_and_log("%s:SUCCESS\n", academic_integrity);
	return;
}

int server_setup(uint16_t port){
	int sockfd, n;
        socklen_t len;

        struct sockaddr_in server_addr, cli_addr;
       // char buffer[1000];

        if (sockfd = socket(AF_INET, SOCK_DGRAM, 0) < 0){
		perror("socket");
		return -1;
	}

        memset(&server_addr, 0 , sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("bind");
		return -1;
	}
	
	return sockfd;
}

void process_input (){
	
	char *command;	
	char buffer[256];
	char *input;
	int n;
	char **tok_argv;
	int tok_argc;

	if (n = read(STDIN, buffer, 256) < 0){
		perror("read error!\n");
		return;
	}

	buffer[n-1] = 0;

	input = (char *)malloc(strlen(buffer)+1);
	strcpy(input, buffer);
	command = strtok (buffer, " \n");
	while(1){

		if (strcasecmp(command, "academic_integrity") == 0){
			academic_integrity();
			break;
		}

		if (strcasecmp(command, "update") == 0){
			update();
			break;
		}

		if (strcasecmp(command, "step") == 0){
			step();
			break;
		}

		if (strcasecmp(command, "packets") == 0){
			printf ("The number of update packets received since last invocation: %d\n", num_packets);
			num_packets = 0;
			break;
		}

		if (strcasecmp(command, "display") == 0){
			display();
			break;
		}

		if (strcasecmp(command, "disable") == 0){
			disable();
			break;
		}

		if (strcasecmp(command, "crash") == 0){
			crash();
			break;
		}

		if (strcasecmp(command, "dump") == 0){
			dump();
			break;
		}

	}

}

