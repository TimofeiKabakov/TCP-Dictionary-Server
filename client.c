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

/* Get sockaddr, IPv4 or IPv6 */
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int main(int argc, char *argv[]){
    int port, numbytes;
    char *hostName, *command;
    char delimiters[] = " ";
    char line[MAXDATASIZE];
    char lineCopy[MAXDATASIZE];
    char bufReceive[MAXDATASIZE * MAXELEMENTS];
    char bufReceiveAdd[MAXDATASIZE];

    /* Allowed commands*/
    char add[] = "add";
    char getvalue[] = "getvalue";
    char getall[] = "getall";
    char remove[] = "remove";
    char quit[] = "quit";

    int sockfd, rv;
    struct addrinfo hints, *servinfo, *iter;

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

    /* Convert port string to a port number */
    port = atoi(argv[2]);
    
    /* CONNECT TO THE SERVER */
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

    /* Check if the port number is within the range allowed */
    if(!(port >= 30001 && port <= 40000)){
        printf("Invalid port number: the port number must be between 30001 and 40000\n");
        exit(1);
    }

    /* Constantly wait for the input from the user */
    while(fgets(line, sizeof(line), stdin)) {
        strncpy(lineCopy, line, sizeof(lineCopy) - 1);
        getRidOfNewLine(lineCopy);

        memset(bufReceive, 0, sizeof(bufReceive));
        memset(bufReceive, 0, sizeof(bufReceiveAdd));

        command = strtok(line, delimiters);
        getRidOfNewLine(command);

        if(!strcmp(command, add) || !strcmp(command, getvalue) || !strcmp(command, getall) || !strcmp(command, remove)) {
            if (send(sockfd, lineCopy, sizeof(lineCopy), 0) == -1) {
                perror("send");
            }

            if (!strcmp(command, getall) || !strcmp(command, getvalue)) {
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
            }

        } else if(!strcmp(command, quit)) {
            close(sockfd);
            exit(1);
        }
    }

    return 0;
}