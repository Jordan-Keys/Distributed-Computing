#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <stdatomic.h>
/*
The above lines are preprocessor directives that include necessary header files for input/output operations, dynamic memory allocation, thread creation and management, and atomic operations.
*/


/*
Here, a structure Node is defined, which represents a node in a binary search tree. 
It contains an atomic integer data to hold the node's value, and pointers left and right to its left and right child nodes.
*/
typedef struct Node {
    _Atomic int data;
    struct Node* left;
    struct Node* right;
} Node;

/*
Another structure ThreadArgs is defined to hold the arguments passed to the search thread. 
It contains an integer data representing the value to be searched and a pointer to the root node of the binary search tree.
*/

typedef struct ThreadArgs {
    int data;
    Node* root;
} ThreadArgs;

/*
The createNode function dynamically allocates memory for a new Node and initializes its data with the given value. 
It sets the left and right child pointers to NULL and returns the newly created node.
*/
Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->left = newNode->right = NULL;
    return newNode;
}

/*
The insertNode function is used to insert a new node into the binary search tree.
It takes a double pointer to the root node and the value to be inserted.
If the root is NULL, it creates a new node using createNode and assigns it as the root. 
Otherwise, it recursively traverses the tree based on the value to find the appropriate position for insertion.
*/
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

/*
The searchNode function is used to search for a node with a given value in the binary search tree. 
It takes the root node and the value to search for. 
If the root is NULL or the value matches the root's data, it returns the root. Otherwise, it recursively traverses the tree based on the value to find the matching node.
*/
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

/*
The searchThread function is the entry point for the search threads. 
It takes a void pointer as an argument, which is then cast to a ThreadArgs pointer.
It extracts the search data and the root node from the ThreadArgs structure. 
Then, it starts a loop that repeatedly searches for the given data in the binary search tree until the result is found or the atomic data value of the root becomes INT_MIN. The atomic_load function is used to safely access the atomic data value. 
If the result is found, it prints the value, otherwise, it prints that the value was not found.
*/
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

//The main function is the entry point of the program. 
//It initializes the root node as NULL and inserts several nodes into the binary search tree using the insertNode function.
int main() {
    Node* root = NULL;

    insertNode(&root, 50);
    insertNode(&root, 35);
    insertNode(&root, 20);
    insertNode(&root, 40);
    insertNode(&root, 70);
    insertNode(&root, 60);
    insertNode(&root, 90);
    insertNode(&root, 45);
    insertNode(&root, 21);
    insertNode(&root, 56);
    insertNode(&root, 30);

    /*
    In this section, an array threads of size 3 is created to hold thread identifiers.
    An array threadArgs of size 3 is also created to hold the arguments for each thread, consisting of the search data and the root node. 
    Three threads are created using pthread_create, and the searchThread function is used as the thread routine. Each thread is passed the address of the corresponding threadArgs element as an argument.
    */
    pthread_t threads[4];
    ThreadArgs threadArgs[4] = {
        {30, root},
        {60, root},
        {90, root},
        {3, root}
    };

    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, searchThread, &threadArgs[i]);
    }
/*
In this part, the atomic store operation is used to set the data value of the root node to INT_MIN. 
This serves as a signal to the search threads to stop searching. 
Then, pthread_join is called to wait for each thread to finish execution.
*/
    atomic_store(&root->data, INT_MIN);

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
