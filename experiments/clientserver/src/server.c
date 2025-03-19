#include "clientserver.h"

#include <stdio.h>
#include <string.h>

static struct server_thread server_thread;

int main () {
    char * response_string = "Hello Urs and Guenti";
    struct response const response = {
        .data = response_string,
        .length = strlen (response_string)
    };
    printf ("Response length: %ld\n", response.length);

    if (server_initialize (&server_thread, 8080, 3, &response))
    {
        printf ("Could not start server.\n");
        return 1;
    }

    printf("Server is running in a separate thread. Press any key to stop.\n");

    // Wait for keypress
    getc (stdin);

    printf ("Stopping server...\n");
    server_destroy (&server_thread);
    printf ("Server stopped.\n");

    return 0;
}