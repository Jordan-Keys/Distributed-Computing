#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "192.168.237.109."
#define PORT 1234

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[256];

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Read user input
    printf("Enter the data value: ");
    fgets(buffer, sizeof(buffer), stdin);

    // Send the data value to the server
    if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }

    // Read the server's response
    if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }

    // Display the server's response
    printf("Server response: %s\n", buffer);

    close(clientSocket);
    return 0;
}
