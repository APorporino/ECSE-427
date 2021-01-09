
#include "rpc.h"

#include "a1_lib.h"

rpc_t RPC_Init(char *host, uint16_t port, int *sockfd) {
    int x = create_server(host, port, sockfd);
    if (x == 0) {
        rpc_t RPC = {host, port, 0};
        return RPC;
    }
    rpc_t RPC_ERROR = {.error = -1};
    return RPC_ERROR;
}

rpc_t RPC_Connect(char *host, uint16_t port, int *sockfd) {
    int x = connect_to_server(host, port, sockfd);
    if (x == 0) {
        rpc_t RPC = {host, port, 0};
        return RPC;
    }
    rpc_t RPC_ERROR = {.error = -1};
    return RPC_ERROR;
}

int RPC_Accept(int sockfd_server, int *sockfd_client) {
    int x = accept_connection(sockfd_server, sockfd_client);
    return x;
}

// void RPC_Close(rpc_t *r);

// void RPC_Call(rpc_t *r, char *name, char *args[ARGS_LENGTH]);

ssize_t send_message(int sockfd, char *buf, size_t len) {
    return send(sockfd, buf, len, 0);
}

ssize_t recv_message(int sockfd, char *buf, size_t len) {
    return recv(sockfd, buf, len, 0);
}
