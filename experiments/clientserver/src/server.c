#include "clientserver.h"

#include <stdio.h>
#include <string.h>

static struct server_thread server_thread;

int main () {
    char const * response = "Hello world";
    uint64_t response_length = strlen (response);
    printf ("Response length: %ld\n", response_length);

    if (initialize_server (&server_thread, 8080, 3, response, response_length))
    {
        printf ("Could not start server.\n");
        return 1;
    }

    printf("Server is running in a separate thread. Press any key to stop.\n");

    // Wait for keypress
    getc (stdin);

    printf ("Stopping server...\n");
    destroy_server (&server_thread);
    printf ("Server stopped.\n");

    return 0;
}