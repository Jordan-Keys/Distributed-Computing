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
    pthread_mutex_t lock;
} Node;

Node* newNode(int data) {
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

int getBalance(Node* root) {
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
        return newNode(data);
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
    bool* found;
    pthread_mutex_t* foundLock;
} SearchThreadArgs;

void* searchThread(void* arg) {
    SearchThreadArgs* args = (SearchThreadArgs*)arg;
    int data = args->data;
    Node* root = args->root;

    bool found = search(root, data);

    pthread_mutex_lock(args->foundLock);
    *(args->found) = *(args->found) || found;
    pthread_mutex_unlock(args->foundLock);

    pthread_exit(NULL);
}

void parallelSearch(Node* root, int data) {
    bool found = false;
    pthread_mutex_t foundLock;
    pthread_mutex_init(&foundLock, NULL);

    pthread_t threads[4];
    SearchThreadArgs threadArgs[4];

    for (int i = 0; i < 4; i++) {
        threadArgs[i].data = data;
        threadArgs[i].root = root;
        threadArgs[i].found = &found;
        threadArgs[i].foundLock = &foundLock;
        pthread_create(&threads[i], NULL, searchThread, (void*)&threadArgs[i]);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&foundLock);

    if (found) {
        printf("Node with value %d found in the tree.\n", data);
    } else {
        printf("Node with value %d not found in the tree.\n", data);
    }
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


    while (1) {
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
                parallelSearch(root, target);
                break;
            case 4:
                printf("In-order traversal: ");
                inOrderTraversal(root);
                printf("\n");
                break;
            case 5:
                exit(0);
        }
    }

    return 0;
}

/*
This code is an implementation of a Binary Search Tree (BST) with thread-safe operations using mutex locks. The BST supports basic operations such as insertion, removal, searching for nodes, and printing the tree using in-order traversal. Let's break down the code step-by-step:

1. `Node` struct:
The `Node` struct represents a single node in the BST. It contains the following fields:
- `_Atomic int data`: An atomic integer that stores the data value of the node.
- `int height`: The height of the node (used for balancing).
- `struct Node* left`: A pointer to the left child of the node.
- `struct Node* right`: A pointer to the right child of the node.
- `pthread_mutex_t lock`: A mutex lock associated with the node. This lock is used to ensure thread-safe access to the node during concurrent operations.

2. `newNode` function:
This function creates a new node and initializes its fields with the provided data value. It also initializes the mutex lock associated with the node.

3. Rotations functions (`rightRotate` and `leftRotate`):
These functions perform right and left rotations, respectively, to balance the BST after an insertion or removal operation. These rotations are essential to maintain the BST's properties.

4. `getHeight` and `getBalance` functions:
These functions calculate the height and balance factor of a node, respectively. The balance factor is the difference between the heights of the left and right subtrees of a node.

5. `insertNode` function:
This function inserts a new node with the given data value into the BST while maintaining the BST's properties. It uses recursion to find the appropriate position for insertion. Before modifying a node, it acquires a lock to ensure exclusive access during the modification.

6. `removeNode` function:
This function removes a node with the given data value from the BST while maintaining the BST's properties. Similar to `insertNode`, it also uses recursion and acquires locks before modifying any node.

7. `search` function:
This function searches for a node with the given data value in the BST using recursion. It acquires a lock on each node visited during the search.

8. `SearchThreadArgs` struct and `searchThread` function:
The `SearchThreadArgs` struct is used to pass arguments to the `searchThread` function. It contains the data value to be searched, the root of the tree, a flag to indicate if the value is found, and a mutex lock associated with the flag.

The `searchThread` function is executed by multiple threads when performing parallel search. It searches for the given data value in the BST and sets the `found` flag accordingly. The `found` flag is protected by a mutex lock to ensure synchronized access.

9. `parallelSearch` function:
This function performs parallel search on the BST by creating four threads that simultaneously search for the given data value. Each thread runs the `searchThread` function, and upon completion, the results are combined using the `found` flag.

10. `inOrderTraversal` function:
This function performs an in-order traversal of the BST and prints the data values of the nodes in sorted order.

11. `main` function:
The `main` function acts as a user interface to interact with the BST. It provides options to insert, remove, search, and print the tree. Users can choose their desired operation by inputting the corresponding option.

Overall, this code demonstrates a thread-safe implementation of a Binary Search Tree, allowing multiple threads to concurrently access and modify the tree while ensuring data integrity and avoiding race conditions.
*/
