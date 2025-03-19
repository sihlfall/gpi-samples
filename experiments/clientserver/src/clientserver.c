#include "clientserver.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_GOTO_ERROR(cond, msg, errlabel) \
    do {                                     \
        if (!(cond)) {                       \
            perror ((msg));                   \
            printf ((msg));                   \
            goto errlabel;                   \
        }                                    \
    } while (0);                               

static inline
void
to_big_endian_64 (char * buffer, size_t x) {
    char * p = &buffer[7];
    * (p--) = x & 0xff; x >>= 8;
    * (p--) = x & 0xff; x >>= 8;
    * (p--) = x & 0xff; x >>= 8;
    * (p--) = x & 0xff; x >>= 8;
    * (p--) = x & 0xff; x >>= 8;
    * (p--) = x & 0xff; x >>= 8;
    * (p--) = x & 0xff; x >>= 8;
    * p = x;
}
    
static
void *
server_run (void * args)
{
    struct server_thread * myself = (struct server_thread *) args;

    myself->server_fd = socket (AF_INET, SOCK_STREAM, 0);
    CHECK_GOTO_ERROR(myself->server_fd != 0, "Socket failed", err_create_socket)

    {
        struct sockaddr_in address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons (myself->port)
        };
        CHECK_GOTO_ERROR(
            bind (myself->server_fd, (struct sockaddr *) &address, sizeof (struct sockaddr_in)) >= 0,
            "Bind failed", err_bind
        )
    }

    CHECK_GOTO_ERROR(
        listen (myself->server_fd, myself->max_connections) >= 0,
        "Listen failed", err_listen
    )

    printf("Server is listening on port %d\n", myself->port);

    while (!myself->request_stop) {
        int new_socket = accept (myself->server_fd, NULL, NULL);
        if (new_socket < 0) {
            if (myself->request_stop) break;
            printf ("Accept failed");
            perror ("Accept failed");
            continue;
        }

        char cmd;
        read (new_socket, &cmd, 1);
        printf ("Received: %c\n", cmd);

        if (cmd == 'a') {
            send (new_socket, myself->response_buffer, myself->response_buffer_length, 0);
        }

        close (new_socket);
    }

err_listen:
err_bind:
    close (myself->server_fd); myself->server_fd = 0;
    printf ("Server is shutting down...\n");
err_create_socket:
    pthread_exit (NULL);
}

static inline
void
set_response (
    struct server_thread * server_thread,
    char const * response, size_t response_length
)
{
    size_t response_buffer_length = sizeof (size_t) + response_length;
    char * response_buffer = (char *) malloc (response_buffer_length);
    /* TODO: Check for error? */

    to_big_endian_64 (response_buffer, response_length);
    memcpy (response_buffer + 8, response, response_length);

    server_thread->response_buffer_length = response_buffer_length;
    server_thread->response_buffer = response_buffer;
}

int
initialize_server (
    struct server_thread * server_thread, uint16_t port, int max_connections,
    char const * response, size_t response_length
)
{
    memset (server_thread, 0, sizeof (struct server_thread));
    set_response (server_thread, response, response_length);
    server_thread->port = port;
    server_thread->max_connections = max_connections;
    if (pthread_create (&server_thread->server_tid, NULL, server_run, server_thread)) {
        perror ("Failed to create server thread");
        free (server_thread->response_buffer);
        return 1;
    }
    return 0;
}

void
destroy_server (struct server_thread * server_thread)
{
    server_thread->request_stop = 1;
    shutdown (server_thread->server_fd, SHUT_RDWR); /* interrupt accept () */
    pthread_join (server_thread->server_tid, NULL);
    free (server_thread->response_buffer);
}
