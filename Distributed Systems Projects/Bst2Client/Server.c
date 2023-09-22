#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdatomic.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct Node {
    _Atomic int data;
    int height;
    struct Node* left;
    struct Node* right;
} Node;

// Function declarations
int max(int a, int b);
int getHeight(Node* node);
Node* createNode(int data);
Node* rightRotate(Node* y);
Node* leftRotate(Node* x);
int getBalance(Node* node);
Node* insertNode(Node* root, int data);
Node* findMinNode(Node* node);
Node* removeNode(Node* root, int data);
void inOrderTraversal(Node* root);

typedef struct ClientThreadArgs {
    int data;
    Node* root;
} ClientThreadArgs;

void* searchThread(void* arg);
void* handleClient(void* client_socket_ptr);
int getBSTHeight(Node* node);

// Function to get the height of the BST
int getBSTHeight(Node* node) {
    return (node == NULL) ? -1 : node->height;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int getHeight(Node* node) {
    return (node == NULL) ? -1 : node->height;
}

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->left = newNode->right = NULL;
    newNode->height = 0;
    return newNode;
}

Node* rightRotate(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;

    return x;
}

Node* leftRotate(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;

    return y;
}

int getBalance(Node* node) {
    return (node == NULL) ? 0 : getHeight(node->left) - getHeight(node->right);
}

Node* insertNode(Node* root, int data) {
    if (root == NULL) {
        return createNode(data);
    }

    if (data < root->data) {
        root->left = insertNode(root->left, data);
    } else if (data > root->data) {
        root->right = insertNode(root->right, data);
    } else {
        return root; // Avoid duplicate values (assuming no duplicates in BST)
    }

    root->height = max(getHeight(root->left), getHeight(root->right)) + 1;

    int balance = getBalance(root);

    // Left Left Case
    if (balance > 1 && data < root->left->data) {
        return rightRotate(root);
    }

    // Right Right Case
    if (balance < -1 && data > root->right->data) {
        return leftRotate(root);
    }

    // Left Right Case
    if (balance > 1 && data > root->left->data) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    // Right Left Case
    if (balance < -1 && data < root->right->data) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
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
        if (root->left == NULL || root->right == NULL) {
            Node* temp = root->left ? root->left : root->right;
            free(root);
            return temp;
        }
        Node* minRight = findMinNode(root->right);
        root->data = minRight->data;
        root->right = removeNode(root->right, minRight->data);
    }

    root->height = max(getHeight(root->left), getHeight(root->right)) + 1;

    int balance = getBalance(root);

    // Left Left Case
    if (balance > 1 && getBalance(root->left) >= 0) {
        return rightRotate(root);
    }

    // Left Right Case
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    // Right Right Case
    if (balance < -1 && getBalance(root->right) <= 0) {
        return leftRotate(root);
    }

    // Right Left Case
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

void inOrderTraversal(Node* root) {
    if (root != NULL) {
        inOrderTraversal(root->left);
        printf("%d ", root->data);
        inOrderTraversal(root->right);
    }
}

void* searchThread(void* arg) {
    ClientThreadArgs* threadArgs = (ClientThreadArgs*)arg;
    int data = threadArgs->data;
    Node* root = threadArgs->root;
    Node* result = NULL;

    int currentHeight = 0;
    const int maxAllowedHeight = getBSTHeight(root);

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

        // If the search reached a leaf node and the current height is less than maxAllowedHeight,
        // assume the target is not in the BST and break out of the loop.
        if (curr == NULL && currentHeight < maxAllowedHeight)
            break;

        // Increment the height of the current subtree being searched.
        currentHeight++;
    }

    if (result != NULL) {
        char* message = (char*)malloc(50 * sizeof(char));
        snprintf(message, 50, "Found %d", result->data);
        return message;
    } else {
        char* message = (char*)malloc(50 * sizeof(char));
        snprintf(message, 50, "%d not found", data);
        return message;
    }
}

void* handleClient(void* client_socket_ptr) {
    int client_socket = *((int*)client_socket_ptr);

    int option, target;
    recv(client_socket, &option, sizeof(option), 0);
    recv(client_socket, &target, sizeof(target), 0);

    Node* root = NULL;
    root = insertNode(root, 50);
    root = insertNode(root, 35);
    root = insertNode(root, 20);
    root = insertNode(root, 40);
    root = insertNode(root, 70);
    root = insertNode(root, 60);
    root = insertNode(root, 90);
    root = insertNode(root, 45);
    root = insertNode(root, 21);
    root = insertNode(root, 56);
    root = insertNode(root, 30);

    ClientThreadArgs threadArgs = {target, root};

    char* response = (char*)searchThread(&threadArgs);

    send(client_socket, response, strlen(response), 0);

    free(response);
    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation error");
        return 1;
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(1234);

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding error");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen error");
        return 1;
    }

    printf("Server started. Waiting for connections...\n");

    while (1) {
        // Accept client connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept error");
            continue;
        }

        printf("New client connected. IP: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Create a new thread to handle the client
        if (pthread_create(&thread_id, NULL, handleClient, &client_socket) < 0) {
            perror("Thread creation error");
            return 1;
        }

        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}
