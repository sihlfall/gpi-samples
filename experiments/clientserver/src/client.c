#include "clientserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    struct response response;

    if (client_make_request("127.0.0.1", 8080, &response)) {
        return 1;
    }

    char * text = calloc(response.length + 1, sizeof (char));
    memcpy (text, response.data, response.length);
    client_destroy_response (&response);

    printf("Text read: %s\n", text);

    return 0;
}
