#include "frontend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSIZE 1024

void parseInput(req *message, char *input) {
    // get the command
    char *token = strtok(input, " ");
    strcpy(message->cmd, token);

    // store params in this array
    char *params[ARGS_LENGTH];

    // fill up the array with params
    int i = 0;
    while (token != NULL) {
        token = strtok(NULL, " ");
        params[i] = token;
        i++;
    }

    // 1 param
    if (i > 1) {
        strcpy(message->param1, params[0]);
    }
    // 2 params
    if (i > 2) {
        strcpy(message->param2, params[1]);
    }
}

int main(int argc, char *argv[]) {
    int sockfd;
    char user_input[BUFSIZE] = {0};
    char server_msg[BUFSIZE] = {0};
    req request;

    char *host = "0.0.0.0";
    int port = 10000;

    // user can input their own host and port. Must input both otherwise default is assumed
    if (argc >= 3) {
        host = argv[1];
        port = atoi(argv[2]);
    }

    // attempt to connect to host and port
    if (RPC_Connect(host, port, &sockfd).error < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        return -1;
    }

    // will either recieve a welcome message or shutdown msg meaning the backend is no longer serving new clients.
    recv_message(sockfd, server_msg, sizeof(server_msg));
    printf("%s\n", (char *)server_msg);

    //backend no longer serving new clients so frontend should terminate
    if (strcmp(server_msg, SHUTDOWN_MSG) == 0) {
        return 0;
    }

    while (strcmp(user_input, "exit\n") && strcmp(user_input, "shutdown\n")) {
        memset(user_input, 0, sizeof(user_input));
        memset(server_msg, 0, sizeof(server_msg));
        // read user input from command line
        printf(">> ");
        fgets(user_input, BUFSIZE, stdin);
        // perform parameter marshalling (group messgae into req object)
        parseInput(&request, user_input);
        // send the input to server - must cast to string first -
        send_message(sockfd, (char *)&request, sizeof(request));
        // receive a msg from the server
        ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        if (byte_count <= 0) {
            break;
        }
        // clear the request object to be able to use it again
        strcpy(request.cmd, "");
        strcpy(request.param1, "");
        strcpy(request.param2, "");
        // print the return message
        printf("%s\n", (char *)server_msg);
    }
    return 0;
}
