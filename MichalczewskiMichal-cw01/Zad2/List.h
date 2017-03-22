#ifndef LIST_H
#define LIST_H

// A list node structure
typedef struct Node{
	
	char* 	name;
	char* 	surname;
	char* 	birth;
	int 	phone;
	char* 	address;
	char* 	email;

	struct Node* prev;
	struct Node* next;

}Node;
 
// List structure
typedef struct List{

	Node* head;
	Node* tail;
	int size;

}List;

// Creates and returns a pointer to the newly allocated list
List* createList();

// Deletes the list and all its nodes
void deleteList(List* list);

// Deletes every occurance of the specified data in the list
void deleteNode(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list);

// Return the number of occurances of given data in the given list
int findNodes(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list);

// Returns the first occurance of given data on the given list, returns NULL if no entries were found
Node* findNode(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list);

// Creates a node with given data and appends it to the beggining of the list
void createAtHead(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list);

// Creates a node with given data and appends it to the tail of the list
void createAtTail(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list);

// Appends the given node at the beggining of the list
void appendAtHead(Node* node, List* list);

// Appends the given node at the end of the list
void appendAtTail(Node* node, List* list);

// Sort the list by name in ascending order, using insertion sort
void sortByName(List* list);

// Prints the list in the ascending order (head to tail)
void printList(List* list);


#endif