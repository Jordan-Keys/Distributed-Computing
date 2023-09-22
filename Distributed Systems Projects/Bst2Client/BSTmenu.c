#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdatomic.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>

typedef struct Node {
    _Atomic int data;
    int height;
    struct Node* left;
    struct Node* right;
    pthread_mutex_t lock; // Mutex lock for thread safety
} Node;

Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->height = 1;
    newNode->left = NULL;
    newNode->right = NULL;
    pthread_mutex_init(&newNode->lock, NULL); // Initialize the mutex lock
    return newNode;
}

int getHeight(Node* root) {
    if (root == NULL) {
        return 0;
    }
    return root->height;
}

int getBalance(Node* root) { //calculates Balance factor
    if (root == NULL) {
        return 0;
    }
    return getHeight(root->left) - getHeight(root->right);
}

Node* rightRotate(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = fmax(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = fmax(getHeight(x->left), getHeight(x->right)) + 1;

    return x;
}

Node* leftRotate(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    // Update heights
    x->height = fmax(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = fmax(getHeight(y->left), getHeight(y->right)) + 1;

    return y;
}

Node* insertNode(Node* root, int data) {
    if (root == NULL) {
        return createNode(data);
    }

    pthread_mutex_lock(&root->lock); // Lock the current node

    if (data < root->data) {
        root->left = insertNode(root->left, data);
    } else if (data > root->data) {
        root->right = insertNode(root->right, data);
    } else {
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return root; // Duplicate keys are not allowed
    }

    root->height = fmax(getHeight(root->left), getHeight(root->right)) + 1;

    int balance = getBalance(root);

    // Left Left Case
    if (balance > 1 && data < root->left->data) {
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return rightRotate(root);
    }

    // Right Right Case
    if (balance < -1 && data > root->right->data) {
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return leftRotate(root);
    }

    // Left Right Case
    if (balance > 1 && data > root->left->data) {
        root->left = leftRotate(root->left);
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return rightRotate(root);
    }

    // Right Left Case
    if (balance < -1 && data < root->right->data) {
        root->right = rightRotate(root->right);
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return leftRotate(root);
    }

    pthread_mutex_unlock(&root->lock); // Unlock the current node
    return root;
}

Node* removeNode(Node* root, int data) {
    if (root == NULL) {
        return root;
    }

    pthread_mutex_lock(&root->lock); // Lock the current node

    if (data < root->data) {
        root->left = removeNode(root->left, data);
    } else if (data > root->data) {
        root->right = removeNode(root->right, data);
    } else {
        if (root->left == NULL || root->right == NULL) {
            Node* temp = root->left ? root->left : root->right;
            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp;
            }
            free(temp);
        } else {
            Node* temp = root->right;
            while (temp->left != NULL) {
                temp = temp->left;
            }
            root->data = temp->data;
            root->right = removeNode(root->right, temp->data);
        }
    }

    if (root == NULL) {
        return root;
    }

    root->height = fmax(getHeight(root->left), getHeight(root->right)) + 1;

    int balance = getBalance(root);

    // Left Left Case
    if (balance > 1 && getBalance(root->left) >= 0) {
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return rightRotate(root);
    }

    // Right Right Case
    if (balance < -1 && getBalance(root->right) <= 0) {
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return leftRotate(root);
    }

    // Left Right Case
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return rightRotate(root);
    }

    // Right Left Case
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return leftRotate(root);
    }

    pthread_mutex_unlock(&root->lock); // Unlock the current node
    return root;
}

bool search(Node* root, int data) {
    if (root == NULL) {
        return false;
    }

    pthread_mutex_lock(&root->lock); // Lock the current node

    if (data == root->data) {
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return true;
    } else if (data < root->data) {
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return search(root->left, data);
    } else {
        pthread_mutex_unlock(&root->lock); // Unlock the current node
        return search(root->right, data);
    }
}

typedef struct SearchThreadArgs {
    int data;
    Node* root;
} SearchThreadArgs;

void* searchThread(void* arg) {
    SearchThreadArgs* args = (SearchThreadArgs*)arg;
    int data = args->data;
    Node* root = args->root;

    bool found = search(root, data);
    char* response = (char*)malloc(50 * sizeof(char)); // Allocate memory for the response message

    if (found) {
        sprintf(response, "Node with value %d found in the tree.", data);
    } else {
        sprintf(response, "Node with value %d not found in the tree.", data);
    }

    pthread_exit(response);
}

void inOrderTraversal(Node* root) {
    if (root != NULL) {
        inOrderTraversal(root->left);
        printf("%d ", root->data);
        inOrderTraversal(root->right);
    }
}

int main() {
    Node* root = NULL;
    int option, target;

    printf("Binary Search Tree Operations:\n");
    printf("1. Insert a node\n");
    printf("2. Remove a node\n");
    printf("3. Search for a node\n");
    printf("4. Print the tree (in-order traversal)\n");
    printf("5. Exit\n");

    while (true) {
        printf("Enter your option: ");
        scanf("%d", &option);

        switch (option) {
            case 1:
                printf("Enter the value to insert: ");
                scanf("%d", &target);
                root = insertNode(root, target);
                break;
            case 2:
                printf("Enter the value to remove: ");
                scanf("%d", &target);
                root = removeNode(root, target);
                break;
            case 3:
                printf("Enter the value to search: ");
                scanf("%d", &target);
                SearchThreadArgs threadArgs = {target, root};
                char* response;
                pthread_t thread;
                pthread_create(&thread, NULL, searchThread, (void*)&threadArgs);
                pthread_join(thread, (void**)&response);
                printf("%s\n", response);
                free(response);
                break;
            case 4:
                printf("In-order traversal: ");
                inOrderTraversal(root);
                printf("\n");
                break;
            case 5:
                return 0;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}
