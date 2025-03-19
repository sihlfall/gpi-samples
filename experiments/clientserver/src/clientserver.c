#include "clientserver.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_GOTO_ERROR(cond, msg, errlabel) \
    do {                                     \
        if (!(cond)) {                       \
            reterr = 1;                      \
            perror ((msg));                  \
            printf ((msg));                  \
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

static inline
size_t
from_big_endian_64 (char const * buffer) {
    char const * p = buffer;
    size_t x = * p++; x <<= 8;
    x |= * p++; x <<= 8;
    x |= * p++; x <<= 8;
    x |= * p++; x <<= 8;
    x |= * p++; x <<= 8;
    x |= * p++; x <<= 8;
    x |= * p++; x <<= 8;
    x |= * p;
    return x;
}

static
void *
server_run (void * args)
{
    __attribute_maybe_unused__ int reterr = 0;
    struct server_thread * myself = (struct server_thread *) args;

    myself->server_fd = socket (AF_INET, SOCK_STREAM, 0);
    CHECK_GOTO_ERROR(myself->server_fd != 0, "Socket failed", err_create_socket)

    {
        int optval = 1;
        setsockopt (myself->server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    }
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
    struct response const * response
)
{
    size_t response_buffer_length = sizeof (size_t) + response->length;
    char * response_buffer = (char *) malloc (response_buffer_length);
    /* TODO: Check for error? */

    to_big_endian_64 (response_buffer, response->length);
    memcpy (response_buffer + 8, response->data, response->length);

    server_thread->response_buffer_length = response_buffer_length;
    server_thread->response_buffer = response_buffer;
}

int
server_initialize (
    struct server_thread * server_thread, uint16_t port, int max_connections,
    struct response const * response
)
{
    memset (server_thread, 0, sizeof (struct server_thread));
    set_response (server_thread, response);
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
server_destroy (struct server_thread * server_thread)
{
    server_thread->request_stop = 1;
    shutdown (server_thread->server_fd, SHUT_RD); /* interrupt accept () */
    pthread_join (server_thread->server_tid, NULL);
    free (server_thread->response_buffer);
}

int
client_make_request(
    char const * hostip4, uint16_t port,
    struct response * response
)
{
    int reterr = 0;
    response->data = NULL; response->length = 0;

    // Create socket
    int sock = socket (AF_INET, SOCK_STREAM, 0);
    CHECK_GOTO_ERROR (sock >= 0, "Socket creation error", err_create_socket)

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons (port)
    };
    CHECK_GOTO_ERROR (
        inet_pton(AF_INET, hostip4, &serv_addr.sin_addr) >= 0,
        "Invalid address/Address not supported", err_convert_address
    )

    CHECK_GOTO_ERROR (
        connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0,
        "Connection failed", err_connect
    )

    char message = 'a';
    send(sock, &message, 1, 0);
    printf("Message sent: %c\n", message);

    ssize_t n_bytes_read;

    /* Read server response */
    char lbuffer[8];
    n_bytes_read = read (sock, &lbuffer, 8);
    printf("%c\n", lbuffer[0]);
    CHECK_GOTO_ERROR(n_bytes_read == 8, "Invalid response", err_receiving_length)
    size_t n_bytes_to_expect = from_big_endian_64 (lbuffer);

    printf("Server response: %lu\n", n_bytes_to_expect);
    response->data = (char *) malloc (n_bytes_to_expect);
    n_bytes_read = read(sock, response->data, n_bytes_to_expect);
    if (n_bytes_read != n_bytes_to_expect) {
        free (response->data); response->data = NULL;
        CHECK_GOTO_ERROR(0, "Invalid response -- too few bytes received", err_inconsistent_response)
    }
    response->length = n_bytes_read;

err_inconsistent_response:
err_receiving_length:
    close(sock);

err_connect:
err_convert_address:
err_create_socket:
    return reterr;
}

void
client_cleanup_response (struct response * response)
{
    free (response->data);
    memset (response, 0, sizeof (*response));
}
