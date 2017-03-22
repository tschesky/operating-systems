#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <time.h>
#include "List.h"


char* generateString(int length){
	char* buff = calloc(length, sizeof(char));
	if (!buff) return NULL;
	for(int j = 0; j < length; j++){
			buff[j] = (char)((rand() % 25) + 65);		
	}
	return buff;
}
int main(){

	List* (*createList)();
	void (*createAtTail)(char*, char*, char*, int, char*, char*, List* );
	void (*sortByName)(List*);
	void (*deleteList)(List*);

	void *handle = dlopen("./libListShared.so", RTLD_LAZY);
	if(!handle){
		printf("Error opening the DL Library!\n");
		return -1;
	}

	createList = dlsym(handle, "createList");
	createAtTail = dlsym(handle, "createAtTail");
	sortByName = dlsym(handle, "sortByName");
	deleteList = dlsym(handle, "deleteList");

	clock_t start_t, tmp1_t, tmp2_t;
   	
	srand(time(NULL));

	struct tms start_time;
	struct tms tmp1;
	struct tms tmp2;

	// STARTING POINT
	start_t = clock();
	printf("MEASURED TIMES AT PROGRAM START:\n");
	if ( times(&start_time) < 0) printf("Problem with times function!\n");
	else {
		printf("\tTotal elapsed time = %fms\n", (double)((start_t)/sysconf(_SC_CLK_TCK))* 1000 );
        printf("\tUser time = %fms\n", (double)((start_time.tms_utime)/sysconf(_SC_CLK_TCK))* 1000 );
        printf("\tSystem time = %fms\n\n",(double)((start_time.tms_stime)/sysconf(_SC_CLK_TCK))* 1000);
     }
     //------------------------------------



	List* lista1 = (*createList)();
	const int noRecords = 30000;

	// CHECK POINT - BEFORE POPULATING THE LIST
	tmp1_t = clock();
	printf("MEASURED TIMES BEFORE POPULATING THE LIST:\n");
	if ( times(&tmp1) < 0) printf("Problem with times function!\n");
	else {
		printf("\tMeasured from the program start:\n");
		printf("\tTotal elapsed time = %fms\n", (((double)(tmp1_t - start_t)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tUser time = %fms\n", (((double)tmp1.tms_utime)/sysconf(_SC_CLK_TCK))* 1000);
        printf("\tSystem time = %fms\n\n",(((double)tmp1.tms_stime)/sysconf(_SC_CLK_TCK))* 1000);
     }
     //------------------------------------
	
	for(int i = 0 ; i < noRecords; i++ ){
		(*createAtTail)(generateString(6), generateString(6), generateString(6), 666666666, generateString(6), generateString(6), lista1);
	}
	
	// CHECK POINT - AFTER POPULATING THE LIST
	tmp2_t = clock();
	printf("MEASURED TIMES AFTER POPULATING THE LIST:\n");
	if ( times(&tmp2) < 0) printf("Problem with times function!\n");
	else {
		printf("\tMeasured from the program start:\n");
		printf("\tTotal elapsed time = %fms\n", (((double)(tmp2_t - start_t)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tUser time = %fms\n", (((double)(tmp2.tms_utime - start_time.tms_utime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tSystem time = %fms\n\n",(((double)(tmp2.tms_stime - start_time.tms_stime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tMeasured from the previous checkpoint:\n");
        printf("\tTotal elapsed time = %fms\n", (((double)(tmp2_t - tmp1_t)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tUser time = %fms\n", (((double)(tmp2.tms_utime - tmp1.tms_utime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tSystem time = %fms\n\n",(((double)(tmp2.tms_stime - tmp1.tms_stime)) / sysconf(_SC_CLK_TCK)) * 1000);

     }
     //------------------------------------


	/*
	printf("List size: %u\n", lista1->size);
	printf("List head: %s, %s\n", lista1->head->name, lista1->head->surname);
	printf("List tail: %s, %s\n", lista1->tail->name, lista1->tail->surname);
	*/

	(*sortByName)(lista1);

	// CHECK POINT - AFTER SORTING THE LIST
	tmp1_t = clock();
	printf("MEASURED TIMES AFTER SORTING THE LIST:\n");
	if ( times(&tmp1) < 0) printf("Problem with times function!\n");
	else {
		printf("\tMeasured from the program start:\n");
		printf("\tTotal elapsed time = %fms\n", (((double)(tmp1_t - start_t)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tUser time = %fms\n", (((double)(tmp1.tms_utime - start_time.tms_utime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tSystem time = %fms\n\n",(((double)(tmp1.tms_stime - start_time.tms_stime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tMeasured from the previous checkpoint:\n");
        printf("\tTotal elapsed time = %fms\n", (((double)(tmp1_t - tmp2_t)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tUser time = %fms\n", (((double)(tmp1.tms_utime - tmp2.tms_utime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tSystem time = %fms\n\n",(((double)(tmp1.tms_stime - tmp2.tms_stime)) / sysconf(_SC_CLK_TCK)) * 1000);
     }
     //------------------------------------



	/*
	printf("List size: %u\n", lista1->size);
	printf("List head: %s, %s\n", lista1->head->name, lista1->head->surname);
	printf("List tail: %s, %s\n", lista1->tail->name, lista1->tail->surname);
	*/

	// CHECK POINT - AFTER DELETING THE LIST
	(*deleteList)(lista1);
	tmp2_t = clock();
	printf("MEASURED TIMES AFTER DELETING THE LIST:\n");
	if ( times(&tmp2) < 0) printf("Problem with times function!\n");
	else {
		printf("\tMeasured from the program start:\n");
		printf("\tTotal elapsed time = %fms\n", (((double)(tmp2_t - start_t)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tUser time = %fms\n", (((double)(tmp2.tms_utime - start_time.tms_utime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tSystem time = %fms\n\n",(((double)(tmp2.tms_stime - start_time.tms_stime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tMeasured from the previous checkpoint:\n");
        printf("\tTotal elapsed time = %fms\n", (((double)(tmp2_t - tmp1_t)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tUser time = %fms\n", (((double)(tmp2.tms_utime - tmp1.tms_utime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tSystem time = %fms\n\n",(((double)(tmp2.tms_stime - tmp1.tms_stime)) / sysconf(_SC_CLK_TCK)) * 1000);
     }


	// END POINT
	return 0;

	dlclose(handle);

}