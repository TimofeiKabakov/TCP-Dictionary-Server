#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10
#define MAXDATASIZE 40
#define MAXELEMENTS 20

typedef struct message {
    char* key;
    char* value;
} MESSAGE;

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

int main(int argc, char *argv[]){
    int numbytes, flag;
    char buf[MAXDATASIZE], interbufSend[MAXDATASIZE];
    char bufSend[MAXDATASIZE * MAXELEMENTS];
    char *command, *new_key;
    char delimiters[] = " ";

    /* Allowed commands*/
    char add[] = "add";
    char getvalue[] = "getvalue";
    char getall[] = "getall";
    char remove[] = "remove";
    char free[] = "free";

    int sockfd, new_fd, i;
    struct addrinfo hints, *servinfo, *iter;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int yes = 1;
    int rv;
    bool keyAlreadyExists = false;

    char* free_key_address = NULL;
    char* free_value_address = NULL;

    /* Storage (Dictionary) */
    MESSAGE dict[MAXELEMENTS];

    /* Check for the right number of arguments */
    if (argc != 2) {
        printf("Incorrect number of arguments: there should be 1 argument\n");
        exit(1);
    }

    /* Check for invalid input (if the port number is in fact a number) */
    if(checkIfNumber(argv[1]) == false) {
        printf("Invalid arguments: the port number should be an integer\n");
        exit(1);
    }

    /* Set up a socket and bind it */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(iter = servinfo; iter != NULL; iter = iter->ai_next) {
        if((sockfd = socket(iter->ai_family, iter->ai_socktype,
        iter->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        /* Get rid of "Address already in use" error */
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
            sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, iter->ai_addr, iter->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
    
        break;
    }

    freeaddrinfo(servinfo);
    
    if (iter == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    /* Listen to incoming requests and accept */
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /* Allocate enough memory */
    for (i = 0; i < MAXELEMENTS; i++) {
        dict[i].key = (char*) malloc(MAXDATASIZE);
        dict[i].value = (char*) malloc(MAXDATASIZE);

        strcpy(dict[i].key, free);
    }

    while(1) {
        int flagGetall;
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
        }

        while(1) {
            memset(bufSend, 0, sizeof(bufSend));
            memset(buf, 0, sizeof(buf));

            numbytes = recv(new_fd, buf, sizeof(buf), 0);

            if (numbytes == -1) {
                perror("recv");
                exit(1);
            }

            if (numbytes == 0) {
                break;
            }

            new_key = (char*) malloc(MAXDATASIZE);
        
            command = strtok(buf, delimiters);
            getRidOfNewLine(command);

            if(!strcmp(command, add)) {
                /* Logic for the add command */
                strcpy(new_key, strtok(NULL, delimiters));
                for (i = 0; i < MAXELEMENTS; i++) {
                    /* Check if there is already a value with the same key */
                    if(!strcmp(dict[i].key, new_key)) {
                        if (send(new_fd, "The key you provided is already associated with a value in the dictionary\n", 75, 0) == -1) { 
                            perror("send");
                        }
                        keyAlreadyExists = true;
                        break;
                    }

                    if(!strcmp(dict[i].key, free) && !free_key_address) {
                        /* Memorize the address of the first free entry */
                        free_key_address = dict[i].key;
                        free_value_address = dict[i].value;
                    }
                }

                if (free_key_address) { 
                    strcpy(free_key_address, new_key);
                    strcpy(free_value_address, strtok(NULL, delimiters));
                    if (send(new_fd, "The provided key-value pair has been added to the dictionary\n", 75, 0) == -1) {
                        perror("send");
                    }

                    /* Reset the free addresses for future lookups */
                    free_key_address = NULL;
                    free_value_address = NULL;
                } else if (!keyAlreadyExists) {
                    if (send(new_fd, "The dictionary is full. Cannot add more key-value pairs\n", 75, 0) == -1) {
                        perror("send");
                    }
                }

                keyAlreadyExists = false;
            } else if(!strcmp(command, getvalue)) {
                /* Logic for the getvalue command */
                flag = 0;
                strcpy(new_key, strtok(NULL, delimiters));
                for (i = 0; i < MAXELEMENTS; i++) {
                    if(!strcmp(dict[i].key, new_key)) {                   
                        sprintf(bufSend, "key: %s, value: %s\n", dict[i].key, dict[i].value);
                        if (send(new_fd, bufSend, sizeof(bufSend), 0) == -1) {
                            perror("send");
                        }
                        memset(bufSend, 0, sizeof(bufSend));
                        flag = 1;
                    }
                }
                if (flag == 0) {
                    if (send(new_fd, "No such element in the dictionary\n", 35, 0) == -1) {
                        perror("send");
                    }
                }
            } else if(!strcmp(command, getall)) {
                /* Logic for the getall command */
                flagGetall = 0;
                sprintf(interbufSend, "Contents of the dictionaty:\n");
                strcat(bufSend, interbufSend);
                for (i = 0; i < MAXELEMENTS; i++) {
                    if(strcmp(dict[i].key, free)) {
                        sprintf(interbufSend, "key: %s, value: %s\n", dict[i].key, dict[i].value);
                        strcat(bufSend, interbufSend);
                        flagGetall = 1;
                    }
                }
                if (flagGetall == 0) {
                    if (send(new_fd, "No elements in the dictionary\n", 35, 0) == -1) { 
                        perror("send");
                    }
                } else {
                    if(send(new_fd, bufSend, sizeof(bufSend), 0) == -1) {
                        perror("send");
                    }
                }
                memset(bufSend, 0, sizeof(bufSend));
            } else if(!strcmp(command, remove)) {
                /* Logic for the remove command */
                strcpy(new_key, strtok(NULL, delimiters));
                for (i = 0; i < MAXELEMENTS; i++) {
                    if(!strcmp(dict[i].key, new_key)) {
                        strcpy(dict[i].key, free);
                    }
                }
                
                getRidOfNewLine(new_key);

                if (send(new_fd, "The key-value pair has been removed\n", 40, 0) == -1) { 
                    perror("send");
                }
            }
        }
    }

    close(new_fd);
    close(sockfd);

    return 0;
}