#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <errno.h>

#define FILE_LOCKED -2

// Function declarations
int rLock(int fileDesc);
int wLock(int fileDesc);
int unlock(int fileDesc);
int listLocks(int fileDesc);
int readByte(int fileDesc);
int writeByte(int fileDesc);
int getLockType(int fileDesc, off_t offset);
int myFcntl(int fileDesc, int cmd, int type, off_t offset, int whence, off_t len);

int main(int argc, char* argv[]){

	// Checking arguments
	if(argc != 2){
		printf("Wrong number of arguments\n");
		return 1;
	}

	// Reading file name
	char* name = argv[1];

	// Opening the file
	int fileDesc = open(name, O_RDWR);
	
	// Handling any problems that may arise
	if(fileDesc < 0){
		printf("Some trouble opening the file\n");
		if( close(fileDesc) < 0){
			printf("Some trouble closing the file");
		}
		return 1;
	}

	int i;
	int result;
	// Main program loop
	do{
		printf("Wprowadź znak:\n"
		   		"\t1) ustawienie rygla do odczytu na wybrany znak pliku\n"
		    	"\t2) ustawienie rygla do zapisu na wybrany znak pliku\n"
		   		"\t3) wyświetlenie listy zaryglowanych znaków pliku\n"
		    	"\t4) zwolnienie wybranego rygla\n"
		    	"\t5) odczyt wybranego znaku pliku\n"
		    	"\t6) zmiana wybranego znaku pliku\n"
				"\t0) wyjscie z programu\n");
		scanf("%d", &i);

		switch(i){
			case 1:
				// Set read lock
				if( (rLock(fileDesc) == -1) ) i = 0;
				break;

			case 2:
				// Set write lock
				if( (wLock(fileDesc) == -1) ) i = 0;
				break;

			case 3:
				// List currently locked bytes on the file
				if( (listLocks(fileDesc) == -1) ) i = 0;
				break;

			case 4: 
				// Unlock a byte
				if( (unlock(fileDesc) == -1) ) i = 0;
				break;
 
			case 5:
				// Read a byte
				result = readByte(fileDesc);
				if(result == -1) i = 0;
				else printf("Odczytany bajt: %c\n", (char)result);
				break;

			case 6:
				if( (writeByte(fileDesc) == -1) ) i = 0;
				break;

			case 0:
				break;

			default:
				printf("Nie dokonales wlasciwego wyboru. Wybierz akcje:\n");
				break;


		}

	}while(i !=0);

	if( close(fileDesc) < 0){
			printf("Some trouble closing the file");
	}

	return 0;
}

// Locking a byte - read lock
int rLock(int fileDesc){
	// Scannig the offset from user
	int where;
	printf("Podaj bajt:  ");
	scanf("%d", &where);
	// If our f-ction returns -1 sth went wrong - we need to check what
	if((myFcntl(fileDesc, F_SETLK, F_RDLCK, where, SEEK_SET, 1)) == -1){
		// We've been locked by another process
		if(errno == EACCES || errno == EAGAIN) printf("Inny proces zalozyl juz rygiel na ten bajt\n");
		else {
			// Program error
			printf("Blad przy zakladaniu rygla(odczyt)\n");
			return -1;
		}
	}
	return 0;
}

// Locking a byta - write lock
int wLock(int fileDesc){
	// Scanning offset from user
	int where;
	printf("Podaj bajt:  ");
	scanf("%d", &where);
	// If our f-ction returns -1 sth went wrong - we need to check what
	if((myFcntl(fileDesc, F_SETLK, F_WRLCK, where, SEEK_SET, 1)) == -1){
		// We've been locked by another process
		if(errno == EACCES || errno == EAGAIN) printf("Inny proces zalozyl juz rygiel na ten bajt\n");
		else {
			// Program error
			printf("Blad przy zakladaniu rygla(zapis)\n");
			return -1;
		}
	}

	return 0;
}

// Unlock a byte
int unlock(int fileDesc){
	// Scanning offset from user
	int where;
	printf("Podaj bajt:  ");
	scanf("%d", &where);
	// If our f-ction returns -1 sth went wrong - we need to check what
	if((myFcntl(fileDesc, F_SETLK, F_UNLCK, where, SEEK_SET, 1)) == -1){
		// We've been locked by another process
		if(errno == EACCES || errno == EAGAIN) printf("Inny proces zalozyl juz rygiel na ten bajt\n");
		else {
			// Program error
			printf("Blad przy zdejmowaniu rygla\n");
			return -1;
		}
	}
	return 0;
}

// List existing locks
int listLocks(int fileDesc){
	int end = 1;
	char* type;
	struct flock lock;
	lock.l_len = 0;
	lock.l_start = 0;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;

	while(end){
		if((fcntl(fileDesc, F_GETLK, &lock)) == -1){
			printf("Blad przy przy pobieraniu informacji o ryglach\n");
			return -1;
		}
		if(lock.l_type != F_UNLCK){
			if(lock.l_type == F_WRLCK)
				type = "zapis";
			else if(lock.l_type == F_RDLCK)
				type = "odczyt";
			else if(lock.l_type == F_WRLCK)
				type = "";

			printf("Proces: %d zablokowal bajt:%d. Rodzaj blokady:%s\n", (int) lock.l_pid, (int) lock.l_start, type);
			lock.l_len = 0;
			lock.l_start++;
			lock.l_whence = SEEK_SET;
		}
		else
			end = 0;
	}
	return 0;
}

int readByte(int fileDesc){
	int where;
	char sign;
	printf("Podaj bajt:  ");
	scanf("%d", &where);
	if((getLockType(fileDesc, where)) == F_WRLCK){
		printf("Inny proces zalozyl juz rygiel na ten bajt\n");
		return FILE_LOCKED;
	}
	if((lseek(fileDesc, where, SEEK_SET)) == -1){
		printf("Blad przy przewijaniu pliku\n");
		return -1;
	}
	if((read(fileDesc, &sign, sizeof(char))) == -1){
		printf("Blad przy czytaniu z pliku\n");
		return -1;
	}

	return (int) sign;
}

// Change a byte in the file
int writeByte(int fileDesc){
	int where;
	char sign;
	printf("Podaj bajt:  ");
	scanf("%d", &where);
	if((getLockType(fileDesc, where)) != F_UNLCK){
		printf("Inny proces zalozyl juz rygiel na ten bajt\n");
		return FILE_LOCKED;
	}
	// Fixing the problem over-input chars in the buffer
	int c;
	while ((c = getchar()) != EOF && c != '\n') ;
	printf("Podaj znak:  ");
	scanf("%c", &sign);
	if((lseek(fileDesc, where, SEEK_SET)) == -1){
		printf("Blad przy przewijaniu pliku\n");
		return -1;
	}
	if((write(fileDesc, &sign, sizeof(char))) == -1){
		printf("Blad przy czytaniu z pliku\n");
		return -1;
	}

	return 0;
}

// Returns the type of lock locked on given byte
int getLockType(int fileDesc, off_t where){
	struct flock lock;
	lock.l_len = 0;
	lock.l_start = where;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;

	if((fcntl(fileDesc, F_GETLK, &lock)) == -1){
		printf("Blad przy uzyskiwaniu informacji o ryglu\n");
		return -1;
	}
	return lock.l_type;
}

// Function for easier usage of fcntl()
int myFcntl(int fileDesc, int cmd, int type, off_t where, int whence, off_t len){
	struct flock lock;

	lock.l_len = len;
	lock.l_start = where;
	lock.l_type = type;
	lock.l_whence = whence;

	return fcntl(fileDesc, cmd, &lock);
}