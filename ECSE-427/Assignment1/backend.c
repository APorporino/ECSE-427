#include "backend.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFSIZE 1024

// constants
char error_msg[BUFSIZE];  // string used to send error messages to clients
char result[BUFSIZE];     // string used to send result messages to clients
char *temp;               // used to send "Error: command 'x' not found" message without \n character"

int main(int argc, char *argv[]) {
    int sockfd, clientfd;
    char msg[BUFSIZE];  // used as temporary string when receiving a message
    req request;
    // boolean values
    int running = 1;          // will control the main backend loop
    int shutdown = 1;         // used to determine if a shutdown command was issued
    int allChildrenDead = 1;  // used to see if all existing children have finsihed before exiting the backend

    char *host = "0.0.0.0";
    int port = 10000;

    // user can input their own host and port. Must input both otherwise default is assumed
    if (argc >= 3) {
        host = argv[1];
        port = atoi(argv[2]);
    }

    // create server
    if (RPC_Init(host, port, &sockfd).error < 0) {
        fprintf(stderr, "oh no\n");
        return -1;
    }

    // will store all pid of child proccess that are running. Allow for 6. 0 means no pid (free spot).
    int childIds[6] = {0, 0, 0, 0, 0, 0};

    while (running) {
        // This causes whole main function to wait untill we have a connection to a client
        if (RPC_Accept(sockfd, &clientfd) < 0) {
            printf("could not accept connection");
            return 0;
        }

        int rval;
        int pid;

        // if we have a connection must fork, so child can handle all further requests and parent can accept more connections
        if ((pid = fork()) == 0) {
            // we only want to serve connection if no shutdown message
            if (shutdown == 1) {
                send_message(clientfd, WELCOME_MSG, strlen(WELCOME_MSG));
                // loop and process the commands demanded
                while ((strcmp(request.cmd, "exit\n") != 0)) {
                    memset(msg, 0, sizeof(msg));
                    // receive a clients message
                    ssize_t byte_count = recv_message(clientfd, msg, sizeof(req));
                    if (byte_count <= 0) {
                        break;
                    }
                    // put the msg sent into request object
                    memcpy(&request, (req *)msg, sizeof(req));
                    if (strcmp(request.cmd, "shutdown\n") == 0) {
                        printf("SHUTDOWN COMMAND ISSUED\n");
                        send_message(clientfd, "Shutting down backend", strlen("Shutting down backend"));
                        return 2;  // return 2 so parent knows we need to shutdown
                    }
                    // will handle the command and send the appropriate message back to client
                    handleCommand(request, clientfd);
                    printf("REQUEST-COMMAND: %s\n", request.cmd);
                }
                printf("A-FRONTEND-ENDED\n");
                return 1;  // return 1 so parent is aware a child proccess ended
            } else {
                // backend has shutdown so we must send appropriate message to frontend so that it ends right away
                send_message(clientfd, SHUTDOWN_MSG, strlen(SHUTDOWN_MSG));
                return 1;
            }
        }
        // this code only executed by the parent process

        // only wish to add clientPIDS that are being served to this array
        if (shutdown == 1) {
            // Add the new pid to childIds array in a free spot
            // Note if no free index is found, client will be handled however will not be able to issue shutdown command
            for (int i = 0; i < 6; i++) {
                if (childIds[i] == 0) {  // we've found a free index
                    childIds[i] = pid;
                    break;
                }
            }
        }

        allChildrenDead = 0;
        // must loop through all 6 possible child processes and check if they ended, or demanded a shutdown.
        for (int i = 0; i < 6; i++) {
            if (childIds[i] != 0) {  // process exists
                allChildrenDead += 1;
                int res = waitpid(childIds[i], &rval, WNOHANG);

                // child process exited
                if (WEXITSTATUS(rval) == 1) {
                    // note that this index only frees up on the following connection.
                    childIds[i] = 0;
                    allChildrenDead -= 1;
                }

                //child process demanded a shutdown
                if (WEXITSTATUS(rval) == 2) {
                    childIds[i] = 0;
                    allChildrenDead -= 1;
                    shutdown = 0;
                }
                rval = 0;  //reset rval
            }
        }
        // backend only ends if shutdown was demanded (shutdown=0) and all children have ended.
        // because we only see when a child ends with a new connection, (allCHildrenDead will be 1 when all are completed)
        if (!shutdown && (allChildrenDead <= 1)) {
            printf("ENDING BACKEND FOREVER\n");
            running = 0;
        }
    }
    return 0;
}

int addInts(int a, int b) {
    return a + b;
}

int multiplyInts(int a, int b) {
    return a * b;
}

float divideFloats(float a, float b) {
    if (b == 0) {
        return -1.0;
    }
    return a / b;
}

int sleepFor(int x) {
    sleep(x);
    return 0;
}

uint64_t factorial(int x) {
    if (x == 0) {
        return 1;
    }
    return x * factorial(x - 1);
}

void handleCommand(req request, int socket) {
    int x = checkParameters(request, socket);
    if (x == 0) {
        performCommand(request, socket);
    }
}

int checkParameters(req request, int socket) {
    strtok(request.cmd, "\n");  // remove new line character if it is there

    if ((strcmp(request.cmd, "add") == 0) | (strcmp(request.cmd, "multiply") == 0) | (strcmp(request.cmd, "divide") == 0)) {
        // check that 2 paramters are present
        if ((strcmp(request.param1, "") == 0) | (strcmp(request.param2, "") == 0)) {
            send_message(socket, "Error: this command requires 2 parameters", strlen("Error: this command requires 2 parameters"));
            return -1;
        }
    } else if ((strcmp(request.cmd, "sleep") == 0) | (strcmp(request.cmd, "factorial") == 0)) {
        // check that 1 parameter is present
        if ((strcmp(request.param1, "") == 0) | (strcmp(request.param1, "\n") == 0)) {
            send_message(socket, "Error: this command requires 1 parameters", strlen("Error: this command requires 1 parameters"));
            return -1;
        }
    }
    return 0;
}

void performCommand(req request, int socket) {
    if (strcmp(request.cmd, "add") == 0) {
        int x = addInts(atoi(request.param1), atoi(request.param2));
        sprintf(result, "%d", x);
        send_message(socket, result, strlen(result));

    } else if (strcmp(request.cmd, "multiply") == 0) {
        int x = multiplyInts(atoi(request.param1), atoi(request.param2));
        sprintf(result, "%d", x);
        send_message(socket, result, strlen(result));
    } else if (strcmp(request.cmd, "divide") == 0) {
        float x = divideFloats(atof(request.param1), atof(request.param2));
        if (x == -1.000000) {
            strcpy(result, "Error: Division by zero");
        } else {
            sprintf(result, "%f", x);
        }
        send_message(socket, result, strlen(result));
    } else if (strcmp(request.cmd, "factorial") == 0) {
        int x = factorial(atoi(request.param1));
        sprintf(result, "%d", x);
        send_message(socket, result, strlen(result));
    } else if (strcmp(request.cmd, "sleep") == 0) {
        sleepFor(atoi(request.param1));
        snprintf(error_msg, sizeof(error_msg), "Slept for %d seconds", atoi(request.param1));
        send_message(socket, error_msg, strlen(error_msg));
    } else if (strcmp(request.cmd, "exit\n") == 0) {
        send_message(socket, "Goodbye", strlen("Goodbye"));
    } else {  // command demanded does not exist
        // remove newline char if there is one
        temp = strtok(request.cmd, "\n");
        // create the error message and store in error_msg var
        snprintf(error_msg, sizeof(error_msg), "Error: Command '%s' not found", temp);
        send_message(socket, error_msg, strlen(error_msg));
    }
}
