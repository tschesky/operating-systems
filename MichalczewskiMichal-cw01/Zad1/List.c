//gcc -ansi -pedantic -std=c99 -W -Wall -o List List.c
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
List* createList(){
	List* newList = (List*)malloc(sizeof(List));
	newList->head = NULL;
	newList->tail = NULL;
	newList->size = 0;

	//printf("Created new list!\n");
	return newList;
}

// Deletes the list and all its nodes
void deleteList(List* list){

	Node* tmp;
	while(list->head != NULL){
		//printf("Deleting node with name: %s\n", list->head->name);
		tmp = list->head->next;
		free(list->head);
		list->size--;
		list->head = tmp;

	}
	free(list);

	//printf("Deleted the list!\n");

}

// Deletes every occurance of the specified data in the list
void deleteNode(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list){

	Node* tmp;
	Node* head = list-> head;
	while(head != NULL){
		// printf("Just while things\n");
		tmp = head->next;
		if(!strcmp(head->name, name) && !strcmp(head->surname, surname) && !strcmp(head->birth, birth) && (head->phone == phone) 
									 && !strcmp(head->address, address) && !strcmp(head->email, email)) {
			if(head->prev != NULL) head->prev->next = head->next;
			else list->head = head->next;
			if(head->next != NULL) head->next->prev = head->prev;
			else list->tail = head->prev;
			//printf("Deleted node with name: %s\n", head->name);
			free(head);
			list->size--;
		}
		head = tmp;
	}

}

// Return the number of occurances of given data in the given list
int findNodes(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list){

	Node* head = list->head;
	int occurance = 0;
	while(head != NULL){
		if(!strcmp(head->name, name) && !strcmp(head->surname, surname) && !strcmp(head->birth, birth) && (head->phone == phone) 
									 && !strcmp(head->address, address) && !strcmp(head->email, email)) {
			occurance++;
		}
		head = head->next;
	}

	return occurance;
}

// Returns the first occurance of given data on the given list, returns NULL if no entries were found
Node* findNode(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list){
	
	Node* head = list->head;
	while(head != NULL){
		if(!strcmp(head->name, name) && !strcmp(head->surname, surname) && !strcmp(head->birth, birth) && (head->phone == phone) 
									 && !strcmp(head->address, address) && !strcmp(head->email, email)) {
			return head;
		}
		head = head->next;
	}

	return NULL;
}

// Creates and returns a pointer to the newly allocated node
Node* createNode(char* name, char* surname, char* birth, int phone, char* address, char* email){
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->name = 		name;
	newNode->surname = 		surname;
	newNode->birth = 		birth;
	newNode->phone = 		phone;
	newNode->address = 		address;
	newNode->email = 		email;
	newNode->prev = 		NULL;
	newNode->next = 		NULL;

	//printf("Created new node!\n");
	return newNode;
}

// Creates a node with given data and appends it to the beggining of the list
void createAtHead(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list){
	Node* newNode = createNode(name, surname, birth, phone, address, email);
	if(list->size == 0){
		list->head = newNode;
		list->tail = newNode;
		list->size++;
		// printf("Created a new node for a list head!\n");
		return;
	}

	list->head->prev = newNode;
	newNode->next = list->head;
	list->head = newNode;
	list->size++;
	// printf("Appended new node at head!\n");
}

// Creates a node with given data and appends it to the tail of the list
void createAtTail(char* name, char* surname, char* birth, int phone, char* address, char* email, List* list){
	Node* newNode = createNode(name, surname, birth, phone, address, email);
	if(list->size == 0){
		list->head = newNode;
		list->tail = newNode;
		list->size++;
		// printf("Created a new node for a list tail!\n");
		return;
	}
	list->tail->next = newNode;
	newNode->prev = list->tail;
	list->tail = newNode;
	list->size++;
	// printf("Appended new node at tail!\n");

}

// Appends the given node at the beggining of the list
void appendAtHead(Node* node, List* list){

	if(list->size ==0){
		list->head = node;
		list->tail = node;
		list->size++;
		// printf("Appended a node as a new list!\n");
		return;
	}
	list->head->prev = node;
	node->next = list->head;
	list->head = node;
	list->size++;
	// printf("Appended a node at head!\n");

}

// Appends the given node at the end of the list
void appendAtTail(Node* node, List* list){

	if(list->size ==0){
		list->head = node;
		list->tail = node;
		list->size++;
		// printf("Appended a node as a new list!\n");
		return;
	}
	list->tail->next = node;
	node->prev = list->tail;
	list->tail = node;
	list->size++;
	// printf("Appended a node at tail!\n");

}


void sortByName(List* list){

	Node* iterator = list->head->next;
	Node* tmp;
	Node* current;
	int i;
	for(i = 1; i < list->size; i++){
		
		//printf("FOR KURWA\n");
		current = iterator;
		tmp = iterator;
		iterator = iterator->next;
		
		while(current->prev != NULL && strcmp(current->name, tmp->name) >= 0){
			//printf("WHILE KURWA ");
			current = current->prev;
		}

		if(current != tmp){
			if(strcmp(current->name, tmp->name) <=0 ){

				if(tmp->next) tmp->next->prev = tmp->prev;
				else list->tail = tmp->prev;
				tmp->prev->next = tmp->next;

				tmp->prev = current;
				tmp->next = current->next;

				current->next->prev = tmp;
				current->next = tmp;

			}else{

				if(tmp->next) tmp->next->prev = tmp->prev;
				else list->tail = tmp->prev;
				tmp->prev->next = tmp->next;

				if(current->prev) current->prev->next = tmp;
				else list->head = tmp;
				tmp->prev = current->prev;
				tmp->next = current;

				current->prev = tmp;

			}


		}

	}

}



// Prints the list in the ascending order (head to tail)
void printList(List* list){

	Node* tmp = list->head;
	while(tmp != NULL){
		printf("Name: %s, Surname: %s, Birth: %s, Telephone: %u, Email: %s\n", tmp->name, tmp->surname, tmp->birth, tmp->phone, tmp->email);
		tmp = tmp->next;
	}
	printf("\n");
}


