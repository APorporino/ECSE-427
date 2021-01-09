/**
 *  @author: Trung Vuong Thien
 *  @email: trung.vuongthien@mail.mcgill.ca
 *  @description: Header file for socket wrapper functions
 */

#ifndef A1_LIB_
#define A1_LIB_

#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BACKLOG_SIZE 10

/**
 *  Set up a server socket.
 *
 *  @params:
 *    host:     A string representing the host address of the server.
 *    port:     An integer in the range [0, 65536) representing the port.
 *    sockfd:   The file descriptor associated with the server socket.
 *  @return:    On success, the file descriptor associated with the newly
 *              created socket is assigned to sockfd, and 0 is returned.
 *              If the function fails to set up the socket, -1 is returned.
 */
int create_server(const char *host, uint16_t port, int *sockfd);

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
int accept_connection(int sockfd, int *clientfd);

/**
 *  Connect to a server.
 *
 *  @params:
 *    host:     A string representing the host address of the server.
 *    port:     An integer in the range [0, 65536) representing the port.
 *    sockfd:   The file descriptor associated with the socket
 *  @return:    On success, the fiel descriptor associated with the newly
 *              created socket is assigned to sockfd, and 0 is returned.
 *              If the function fails to set up the socket, -1 is returned.
 */
int connect_to_server(const char *host, uint16_t port, int *sockfd);

#endif  // A1_LIB_
