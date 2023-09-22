#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Client request codes
#define SEARCH_TARGET 1
#define ADD_NODE 2
#define REMOVE_NODE 3

// Server address and port
#define SERVER_IP "192.168.237.109"
#define SERVER_PORT 1234

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    int option, target;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation error");
        return 1;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection error");
        return 1;
    }

    printf("Connected to server.\n");

    // Client interaction loop
    while (1) {
        printf("\nOptions:\n");
        printf("1. Search for target\n");
        printf("2. Add a node\n");
        printf("3. Remove a node\n");
        printf("4. Exit\n");
        printf("Enter your choice (1/2/3/4): ");
        scanf("%d", &option);

        if (option == SEARCH_TARGET || option == ADD_NODE || option == REMOVE_NODE) {
            printf("Enter the target value: ");
            scanf("%d", &target);

            // Send the option and target value to the server
            send(client_socket, &option, sizeof(option), 0);
            send(client_socket, &target, sizeof(target), 0);

            // Wait for the server's response
            char buffer[1024];
            recv(client_socket, buffer, sizeof(buffer), 0);
            printf("Server response: %s\n", buffer);
        } else if (option == 4) {
            // Exit the client
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid option. Please enter a valid option (1/2/3/4).\n");
        }
    }

    close(client_socket);
    return 0;
}