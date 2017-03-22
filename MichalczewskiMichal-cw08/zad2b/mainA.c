#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define RECORD_SIZE 1024

pthread_t * ids; //identyfikatory wątków
char * plik = "test.txt";
int fl; //plik z rekordami
bool threadFlag = false; //flaga utworzenia wątków
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //standardowa blokada mutex
int iloscRekordow = 3; //ilość rekordów czytanych przez wątek
int iloscWatkow = 4; //ilosc uruchomionych wątków
char *slowo = "bi";
bool dzieleniePrzezZero = false;

struct sigaction sigact; //nowa akcja dla sygnału

void *threadFunc(void *);
void atexitFunc();


void handler(int, siginfo_t *, void *);

struct rekord{
	int id;
	char * txt;
}; // struktura do przechowywania rekordów

int main(int argc, char const *argv[])
{
	atexit(atexitFunc);
	int c;

	sigact.sa_sigaction = handler; //przypisanie acji handler dla sygnału

	ids = malloc(sizeof(pthread_t)*iloscWatkow);

	fl = open(plik,O_RDONLY);
	if(fl == -1){
		printf("Blad otwierania pliku\n%s\n",strerror(errno));
		exit(-1);
	}

	for(int i=0; i<iloscWatkow; i++){
			//tworzenie wątków
						//id,atrybuty,funkcja,arg funkcji
		c = pthread_create(ids+i,NULL,threadFunc,slowo);
		if(c!=0){
			printf("Blad podczas tworzenia watku nr %d\n%s\n",i,strerror(c));
			exit(-1);
		}
	}
	sigaction(SIGFPE, &sigact, NULL); //obsługa sygnału dpowiadającego m.in. za dzielenie przez zero
	threadFlag = true;


	for(int i=0; i<iloscWatkow; i++){
			//czekanie aż wątki zakończą co mają do zrobienia
		c = pthread_join(ids[i],NULL);
		if(c!=0){
			printf("Blad podczas joinowania watku nr %d\n%s\n",i,strerror(c));
			exit(-1);
		}

	}

	return 0;
}

void * threadFunc(void * slowo){

	int czytaj = 1;
	struct rekord bufor[iloscRekordow]; //tablica rekordów
	int c; 
	 					//może być odwołany w każdej chwili
	c = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	if(c!=0) {
		printf("Blad podczas ustawiania typu\n%s\n",strerror(c));
		exit(-1);
	}



	while(!threadFlag); //czekanie na utworzenie wątków

	char *tmp = malloc(sizeof(char)*RECORD_SIZE); //zmienna tymczasowa na identyfikator rekordu
	char *strtmp = malloc(sizeof(char)*RECORD_SIZE);
	char *nmb = malloc(sizeof(char)*3);
	int j; 
	int k;

	while(czytaj>0) { //czytaj = 0 na końcu pliku

		pthread_mutex_lock(&mutex); //blokowanie pliku

		if(!dzieleniePrzezZero){
			printf("Watek %lu dzieli przez zero",(unsigned long)pthread_self());
			dzieleniePrzezZero = true;
			int num = 1/0;
		}
		
		for (int i = 0; i<iloscRekordow; i++) {
			j = 0;
			czytaj = read(fl,tmp,RECORD_SIZE+1);
			if(czytaj == -1){
				printf("Blad podczas czytania z pliku\n%s\n",strerror(errno));
				exit(-1);			
				}

			while((int)tmp[j]!=32) { // 32 = spacja
				nmb[j] = tmp[j];
				j++;
			} //przeczytano id rekordu
			bufor[i].id = atoi(nmb);
			bufor[i].txt = malloc(sizeof(char)*(RECORD_SIZE));
			j++; //przecodzę do pierwszego znaku tekstu rekordu
			k = 0;
			while((int)tmp[j]!=0){ // 0 = koniec napisu
				strtmp[k] = tmp[j];
				j++;
				k++;
			} //przeczytano tekst
			j++;
			strcpy(bufor[i].txt,strtmp); 
		}
		
		pthread_mutex_unlock(&mutex); //zwalnianie blokady mutexu

		char * szukacz; // wskaznik na znalezione wystapienie danego slowa w buforze
		for(int i=0; i<iloscRekordow; i++) {
			szukacz = strstr(bufor[i].txt,slowo);
			if (szukacz != NULL){ //znaleziono
				for (int l = 0; l < iloscWatkow; l++) { //zamykanie pozostałych wątków 
					if(ids[l]!=pthread_self()){
						pthread_cancel(ids[l]);
					}
				}
				printf("Watek o TID: %lu Znalazl slowo w rekordzie o identyfikatorze %d\n",(unsigned long)pthread_self(),bufor[i].id);
				exit(1);
			}
		}
	} //koniec while'a
	return NULL;
}
void handler(int sign, siginfo_t *info, void *ptr) {
	pid_t tid = syscall(SYS_gettid); //pobieranie unikalnego ID dla wątku
	printf("Zlapalem sygnal: %d, TID: %ld\n",sign,(long)tid);
}

void atexitFunc(){
	free(ids);
	close(fl);
}