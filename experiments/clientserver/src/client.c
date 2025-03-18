#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

char * response;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *message = "a";
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send message to server
    send(sock, message, strlen(message), 0);
    printf("Message sent: %s\n", message);

    // Read server response
    response = NULL;
    uint64_t response_length = 0;
    ssize_t valread = read(sock, &response_length, sizeof(uint64_t));
    if (valread == 8) {
        printf("Server response: %lu\n", response_length);
        response = calloc(response_length + 1, sizeof(char));
        valread = read(sock, response, response_length);
        printf("%lu\n", valread);
        response[response_length] = 0;
    }

    printf("Text read: %s\n", response);

    // Close socket
    close(sock);

    return 0;
}
