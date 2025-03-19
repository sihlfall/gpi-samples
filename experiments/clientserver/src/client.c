#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

static inline
size_t from_big_endian_64 (char const * buffer) {
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

void
make_request(
    char const * hostip4, uint16_t port,
    char ** response, size_t * response_length
)
{
    // Create socket
    int sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons (port)
    };
    if (inet_pton(AF_INET, hostip4, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char message = 'a';
    send(sock, &message, 1, 0);
    printf("Message sent: %c\n", message);

    ssize_t n_bytes_read;

    /* Read server response */
    char lbuffer[8];
    n_bytes_read = read (sock, &lbuffer, 8);
    printf("%c\n", lbuffer[0]);
    if (n_bytes_read != 8) {
        perror ("Invalid response");
        exit (EXIT_FAILURE);
    }
    size_t n_bytes_to_expect = from_big_endian_64 (lbuffer);

    printf("Server response: %lu\n", n_bytes_to_expect);
    *response = (char *) malloc (n_bytes_to_expect);
    n_bytes_read = read(sock, *response, n_bytes_to_expect);
    if (n_bytes_read != n_bytes_to_expect) {
        perror ("Invalid response -- too few bytes received");
        exit (EXIT_FAILURE);
    }
    *response_length = n_bytes_read;

    // Close socket
    close(sock);
}

int main() {
    char * response = NULL;
    size_t response_length = 0;

    make_request("127.0.0.1", 8080, &response, &response_length);

    char * text = calloc(response_length + 1, sizeof (char));
    memcpy (text, response, response_length);

    free (response);

    printf("Text read: %s\n", text);

    return 0;
}
