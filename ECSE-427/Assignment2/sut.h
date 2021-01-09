#ifndef __SUT_H__
#define __SUT_H__
#include <pthread.h>
#include <stdbool.h>
#include <ucontext.h>
#include <unistd.h>
#include <stdio.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SLEEP_TIME                         1000
#define MAX_THREADS                        1000
#define THREAD_STACK_SIZE                  1024*64

extern struct queue taskQ;
extern struct queue waitQ;
extern int numthreads, curthread;


typedef void (*sut_task_f)();

void sut_init();
bool sut_create(sut_task_f fn);
void sut_yield();
void sut_exit();
void sut_open(char *dest, int port);
void sut_write(char *buf, int size);
void sut_close();
char *sut_read();
void sut_shutdown();

/**
 *  Connect to a server. Function taken from Assignment 1 a1_lib.c
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

/**
 *  Send a message on a socket. Function taken from Assignment 1 a1_lib.c
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
 *  Receive a message from a socket. Function taken from Assignment 1 a1_lib.c
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
