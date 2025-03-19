#ifndef CLIENTSERVER_H_
#define CLIENTSERVER_H_

#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>

struct server_thread {
    pthread_t server_tid;
    atomic_int request_stop;
    atomic_int is_running;
    int server_fd;
    uint16_t port;
    int max_connections;
    char * response_buffer;
    size_t response_buffer_length;
};

struct response {
    char * data;
    size_t length;
};

int server_initialize (
    struct server_thread * server_thread, uint16_t port, int max_connections,
    struct response const * response
);

void server_destroy (struct server_thread * server_thread);

int client_make_request (
    char const * hostip4, uint16_t port,
    struct response * response
);

void client_cleanup_response (struct response * response);

#endif