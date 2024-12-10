#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MAXDATASIZE 40
#define MAXELEMENTS 20

/* Checks if provided argument is a number */
bool checkIfNumber(char* arg) {
    int i, n = strlen(arg);
    for(i = 0; i < n; i++) {
        if (arg[i] < '0' || arg[i] > '9') {
            return false;
        }
    }
    return true;
}

/* Get rid of the newline character in a given string */
void getRidOfNewLine(char* str) {
    int i, n = strlen(str);
    for(i = 0; i < n; i++) {
        if(str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

/* Check if the provided command is from the allowed lists */
bool isCommandValid(char* command, char* validCommands[], size_t size) {
    size_t i;
    bool is_command_valid = false;
    for(i = 0; i < size; i++) {
        if(!strcmp(command, validCommands[i])) {
            is_command_valid = true;
            break;
        }
    }
    return is_command_valid;
}

int main(int argc, char *argv[]){
    int numbytes;
    char *hostName, *command;
    char delimiters[] = " ";
    char line[MAXDATASIZE];
    char lineToSend[MAXDATASIZE];
    char bufReceive[MAXDATASIZE * MAXELEMENTS];
    char bufReceiveAdd[MAXDATASIZE];    

    char *validCommands[5];

    int sockfd, rv;
    struct addrinfo hints, *servinfo, *iter;

    /* Allowed commands*/
    char quit[] = "quit";
    validCommands[0] = "add";
    validCommands[1] = "getvalue";
    validCommands[2] = "getall";
    validCommands[3] = "remove";
    validCommands[4] = quit;

    /* Check for the right number of arguments */
    if (argc != 3) {
        printf("Incorrect number of arguments: there should be 2 arguments\n");
        exit(1);
    }

    /* Check for invalid input (if the port number is in fact a number) */
    if(checkIfNumber(argv[2]) == false) {
        printf("Invalid arguments: the port number should be an integer\n");
        exit(1);
    }

    hostName = argv[1];
    
    /* Connect to the server */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostName, argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(iter = servinfo; iter != NULL; iter = iter->ai_next) {
        if((sockfd = socket(iter->ai_family, iter->ai_socktype,
        iter->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if(connect(sockfd, iter->ai_addr, iter->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (iter == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    /* Constantly wait for the input from the user */
    while(fgets(line, sizeof(line), stdin)) {
        strncpy(lineToSend, line, sizeof(lineToSend) - 1);
        getRidOfNewLine(lineToSend);

        memset(bufReceive, 0, sizeof(bufReceive));
        memset(bufReceive, 0, sizeof(bufReceiveAdd));

        command = strtok(line, delimiters);
        getRidOfNewLine(command);

        if(isCommandValid(command, validCommands, 5)) {
            if (!strcmp(command, quit)) {
                close(sockfd);
                exit(1);
            }
            
            /* Send the line command to the server */
            if (send(sockfd, lineToSend, sizeof(lineToSend), 0) == -1) {
                perror("send");
            }

            /* Get back a response with output */
            numbytes = recv(sockfd, bufReceive, sizeof(bufReceive), 0);

            if (numbytes == -1) {
                perror("recv");
                exit(1);
            }

            if (numbytes == 0) {
                printf("Connection closed\n");
                exit(1);
            }

            printf("%s", bufReceive);
        } else {
            printf("Command is not valid\n");
        }
    }

    return 0;
}