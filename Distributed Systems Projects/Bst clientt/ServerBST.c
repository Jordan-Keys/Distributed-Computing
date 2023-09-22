#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <stdatomic.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

typedef struct Node {
    _Atomic int data;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct ThreadArgs {
    int data;
    Node* root;
} ThreadArgs;

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->left = newNode->right = NULL;
    return newNode;
}

void insertNode(Node** root, int data) {
    if (*root == NULL) {
        *root = createNode(data);
        return;
    }
    if (data < (*root)->data) {
        insertNode(&(*root)->left, data);
    } else if (data > (*root)->data) {
        insertNode(&(*root)->right, data);
    }
}

Node* searchNode(Node* root, int data) {
    if (root == NULL || root->data == data) {
        return root;
    }
    if (data < root->data) {
        return searchNode(root->left, data);
    } else {
        return searchNode(root->right, data);
    }
}

Node* findMinNode(Node* node) {
    Node* current = node;
    while (current && current->left != NULL) {
        current = current->left;
    }
    return current;
}

Node* removeNode(Node* root, int data) {
    if (root == NULL) {
        return root;
    }
    if (data < root->data) {
        root->left = removeNode(root->left, data);
    } else if (data > root->data) {
        root->right = removeNode(root->right, data);
    } else {
        if (root->left == NULL) {
            Node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
            free(root);
            return temp;
        }
        Node* minRight = findMinNode(root->right);
        root->data = minRight->data;
        root->right = removeNode(root->right, minRight->data);
    }
    return root;
}

void* searchThread(void* arg) {
    ThreadArgs* threadArgs = (ThreadArgs*)arg;
    int data = threadArgs->data;
    Node* root = threadArgs->root;
    Node* result = NULL;

    while (1) {
        Node* curr = root;
        while (curr != NULL) {
            if (data == curr->data) {
                result = curr;
                break;
            } else if (data < curr->data) {
                curr = curr->left;
            } else {
                curr = curr->right;
            }
        }

        if (result != NULL)
            break;

        if (atomic_load(&root->data) == INT_MIN)
            break;
    }

    if (result != NULL) {
        printf("Found %d\n", result->data);
    } else {
        printf("%d not found\n", data);
    }
    return NULL;
}

void* clientHandler(void* arg) {
    int clientSocket = *((int*)arg);
    char buffer[256];

    if (read(clientSocket, buffer, sizeof(buffer)) < 0) {
        perror("Failed to read data from client");
        close(clientSocket);
        return NULL;
    }

    int data = atoi(buffer);

    Node* root = NULL;
    insertNode(&root, 50);
    insertNode(&root, 35);
    insertNode(&root, 20);
    insertNode(&root, 40);
    insertNode(&root, 70);
    insertNode(&root, 60);
    insertNode(&root, 90);

    Node* result = searchNode(root, data);

    memset(buffer, 0, sizeof(buffer));
    if (result != NULL) {
        snprintf(buffer, sizeof(buffer), "Node with data %d found.", data);
    } else {
        snprintf(buffer, sizeof(buffer), "Node with data %d not found.", data);
    }

    if (write(clientSocket, buffer, strlen(buffer)) < 0) {
        perror("Failed to send response to client");
    }

    close(clientSocket);
    return NULL;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int addrlen = sizeof(serverAddress);
    pthread_t threadId;

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int reuseAddr = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(int)) < 0) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(1234);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Listening for client connections...\n");

    while (1) {
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&threadId, NULL, clientHandler, &clientSocket) < 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
