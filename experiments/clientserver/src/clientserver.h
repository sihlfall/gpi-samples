#ifndef CLIENTSERVER_H_
#define CLIENTSERVER_H_

#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>

struct server_thread {
    pthread_t server_tid;
    atomic_int request_stop;
    int server_fd;
    uint16_t port;
    int max_connections;
    char * response_buffer;
    size_t response_buffer_length;
};

int initialize_server (
    struct server_thread * server_thread, uint16_t port, int max_connections,
    char const * response, size_t response_length
);

void destroy_server (struct server_thread * server_thread);

#endif