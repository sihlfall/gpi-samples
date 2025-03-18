#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>

#define PORT 8080
#define BUFFER_SIZE 1024

atomic_int server_running = 1;
int server_fd;
char *response = "Hello world";
uint64_t response_length = 11;

char * response_buffer;

void *server_thread(void *arg) {
    response_buffer = (char *)malloc(sizeof(uint64_t) + response_length);
    memcpy(response_buffer, &response_length, sizeof(uint64_t));
    memcpy(response_buffer + sizeof(uint64_t), response, response_length);

    int new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        pthread_exit(NULL);
    }

    // Bind the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        pthread_exit(NULL);
    }

    printf("Server is listening on port %d\n", PORT);

    while (server_running) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (!server_running) break;
            perror("Accept failed");
            continue;
        }

        // Read data from the client
        read(new_socket, buffer, BUFFER_SIZE);
        printf("Received: %s\n", buffer);

        // Check if the received message is 'a'
        if (buffer[0] == 'a' && buffer[1] == '\0') {
            send(new_socket, response_buffer, sizeof(uint64_t) + response_length, 0);
        }

        // Close the connection
        close(new_socket);
    }

    close(server_fd);
    printf("Server is shutting down...\n");
    pthread_exit(NULL);
}

void stop_server() {
    server_running = 0;
    shutdown(server_fd, SHUT_RDWR); // Interrupt accept()
}

int main() {
    pthread_t server_tid;
    signal(SIGINT, stop_server); // Handle Ctrl+C to stop the server

    if (pthread_create(&server_tid, NULL, server_thread, NULL) != 0) {
        perror("Failed to create server thread");
        return 1;
    }

    printf("Server is running in a separate thread. Press Ctrl+C to stop.\n");

    // Wait for keypress
    getc(stdin);

    printf("Stopping server...\n");
    stop_server();
    pthread_join(server_tid, NULL);
    printf("Server stopped.\n");

    return 0;
}