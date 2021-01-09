#ifndef MESSAGE
#define MESSAGE
#include <stdint.h>
#include <stdio.h>

#define CMD_LENGTH 256
#define ARGS_LENGTH 25
#define SHUTDOWN_MSG "Backend has shutdown"
#define WELCOME_MSG "Welcome!"

typedef struct request_message {
    char cmd[CMD_LENGTH];
    char param1[CMD_LENGTH];
    char param2[CMD_LENGTH];
} req;

// if error is negative we have no set it up correctly
typedef struct rpc_t {
    char *host;
    int port;
    int error;
} rpc_t;

/**
 *  Set up a server socket.
 *
 *  @params:
 *    host:     A string representing the host address of the server.
 *    port:     An integer in the range [0, 65536) representing the port.
 *    sockfd:   The file descriptor associated with the server socket.
 *  @return:    On success, the file descriptor associated with the newly
 *              created socket is assigned to sockfd, and a rpc_t struct is returned.
 *              If the function fails to set up the socket, rpc_t struct with error is returned.
 */
rpc_t RPC_Init(char *host, uint16_t port, int *sockfd);

/**
 *  Connect to a server.
 *
 *  @params:
 *    host:     A string representing the host address of the server.
 *    port:     An integer in the range [0, 65536) representing the port.
 *    sockfd:   The file descriptor associated with the socket
 *  @return:    On success, the fiel descriptor associated with the newly
 *              created socket is assigned to sockfd, and a rpc_t struct is returned.
 *              If the function fails to set up the socket, rpc_t struct with error is returned.
 */
rpc_t RPC_Connect(char *host, uint16_t port, int *sockfd);

/**
 *  Accept a client connection on a server socket.
 *
 *  @params:
 *    sockfd:   The file descriptor of the server socket.
 *    clientfd: The file descriptor of the client connection.
 *  @return:    On success, a connection is set up between the server and
 *              client processes. The function assigns the file descriptor
 *              value to clientfd, and returns 0. If an error occurs, the
 *              function will return -1.
 */
int RPC_Accept(int sockfd_server, int *sockfd_client);

// void RPC_Close(rpc_t *r);

// void RPC_Call(rpc_t *r, char *name, char *args[ARGS_LENGTH]);

/**
 *  Send a message on a socket.
 *
 *  @params:
 *    sockfd:   The file descriptor of the socket.
 *    buf:      The message to send.
 *    len:      Number of bytes to send.
 *  @return:    On success, the functions returns the number of bytes send.
 *              On error, -1 is returned.
 */
ssize_t send_message(int sockfd, char *buf, size_t len);

/**
 *  Receive a message from a socket.
 *
 *  @params:
 *    sockfd:   The file descriptor of the socket.
 *    buf:      A buffer to store the received message.
 *    len:      The size of the buffer.
 *  @return:    On success, the function returns he number of bytes received.
 *              A value of 0 means the connection on this socket has been
 *              closed. On error, -1 is returned.
 */
ssize_t recv_message(int sockfd, char *buf, size_t len);

#endif