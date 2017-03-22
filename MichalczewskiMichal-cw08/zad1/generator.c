#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define LINES 100
#define MAXSIZE 1024

int main(int argc, char const *argv[])
{

	char* nazwa = "test.txt";
	FILE *f;
	f = fopen(nazwa,"w");

	char alf[] = "qwertyuioplkjhgfdsazxcvbnm";
	char* rekord;
	srand((unsigned) time(NULL));
	int x;
	int tmp;
	
	for(int i = 1; i < LINES+1; i++){
		tmp = i;
		x = 0;
		rekord = malloc(sizeof(char)*(MAXSIZE-x));
		while(tmp > 0){
			tmp = tmp/10;
			++x;
		}
		for (int j = 0; j < (MAXSIZE-x); j++){ 
			rekord[j] = alf[rand()%(sizeof(alf)-1)];
		}
		rekord[MAXSIZE-x-1] = '\0';

		fprintf(f,"%d",i);
		fputs(" ",f);
		fputs(rekord,f);
		fputs("\n",f);
		free(rekord);
	}
	fclose(f);
} 