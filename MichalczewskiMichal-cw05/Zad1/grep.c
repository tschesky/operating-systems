#define _GNU_SOURCE  

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>

// Function for getting the type of the file in a char format (directory, regular file etc), output is saved in a variable, passed by a pointer
void getMode(char *out, struct stat statbuf);

// Main program process does wc -l command
int main(int argc, char* argv[]){

	// Array for file descriptors, first pipe
	int fd1[2];

	// Piping and forking
    if (pipe(fd1) == -1 ){
    	printf("Error while creating anonymous pipe!\n");
    	return 1;
    }
    pid_t childpid1 = fork();

    // Return status no. 1 for waiting for first child process
    int returnStatus_1;  
    // First child process executes grep ^d command
    if(childpid1 == -1){
    	printf("Error while forking process 1!\n");
    	return 1;
    }
  	else if(childpid1 == 0)
 	{
 		// Array for descriptors, second pipe
  	    int fd2[2];

  	    // Piping and forking
	     if (pipe(fd2) == -1 ){
    		printf("Error while creating anonymous pipe!\n");
	    	return 1;
	    }
	    pid_t childpid2 = fork();

	    // Tmp for file permissions, line for writing to file
	    char* tmp = malloc(128);
    	char* line = malloc(300);

    	// Return status no. 2, for waiting for second child process
    	int returnStatus_2;  

	    // Second child process executes ls -l command
	    if(childpid2 == -1){
	    	printf("Error while forking process 2!\n");
	    	return 1;
    	}
	  	else if(childpid2 == 0)
	 	{
	  	    // Child closes the file for reading, we only want to pass ls -l result further
	        close(fd2[0]);

	        // Directory pointer, stats for reading directory and passwd and group for getting names
		    DIR *dPointer;
		    struct dirent *entry;
		    struct stat statbuf;
		    struct passwd *pwd;
		    struct group *gr;
		    char out = 'a';

		    // Hadling errors for opening the directory
		    if((dPointer = opendir(".")) == NULL) {
		        fprintf(stderr,"cannot open directory!\n");
		        return 1;
		    }

		    // Iterating over the directory
		    while((entry = readdir(dPointer)) != NULL) {
	    	
	    		// Ignore . and ..
	    		if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0) continue;

		    	// Putting info into statbuf
		        stat(entry->d_name,&statbuf);

		        // Building permission string
			    tmp[0] = ((statbuf.st_mode & S_IRUSR) ? 'r' : '-');
			    tmp[1] = ((statbuf.st_mode & S_IWUSR) ? 'w' : '-');
			    tmp[2] = ((statbuf.st_mode & S_IXUSR) ? 'x' : '-');
			    tmp[3] = ((statbuf.st_mode & S_IRGRP) ? 'r' : '-');
			    tmp[4] = ((statbuf.st_mode & S_IWGRP) ? 'w' : '-');
			    tmp[5] = ((statbuf.st_mode & S_IXGRP) ? 'x' : '-');
			    tmp[6] = ((statbuf.st_mode & S_IROTH) ? 'r' : '-');
			    tmp[7] = ((statbuf.st_mode & S_IWOTH) ? 'w' : '-');
			    tmp[8] = ((statbuf.st_mode & S_IXOTH) ? 'x' : '-');    

			    // Getting user and group info from ID's 
			    pwd = getpwuid(statbuf.st_uid);
			    gr = getgrgid(statbuf.st_gid);

			    // Getting file mode
			    getMode(&out, statbuf);

			    // Time operations, we get the time, get it's local value and strftime() it to desired format
				time_t t = statbuf.st_mtime;
				struct tm* lt;
				lt = localtime(&t);
				char timbuf[80];
				strftime(timbuf, sizeof(timbuf), "%b %d %H:%M", lt);

				// Writing to file
				sprintf(line, "%c%s %d %s %s %6d %s %s\n", out, tmp, (int)statbuf.st_nlink, pwd->pw_name, gr->gr_name, (int)statbuf.st_size, timbuf, entry->d_name);
				write(fd2[1], line, strlen(line));

    		}
    		// Close directory pointer, exit procces with success status
		    closedir(dPointer);
		    _exit(0);
		}
	  	else
	  	{
	        // Close the file responsible for writing in pipe with child
	        close(fd2[1]);  
	        // CLose the file responsible for reading in pipe with parent
        	close(fd1[0]);

        	// Wait for child to terminate
       		waitpid(childpid2, &returnStatus_2, 0);

       		// TO DO - STATUS CHECK

       		// Tmp variables for reading 
       		char * line = NULL;
		    size_t len = 0;
		    ssize_t read;

		    // Pointer for easier file reading
		    FILE* fp = fdopen(fd2[0], "r");

		    // Reading from file, checking the regexp and writing to another
	       	while ( (read = getline(&line, &len, fp)) != -1) {
	        	if(line[0] == 'd') write(fd1[1], line, strlen(line));
	       	}

	       	// Close pointer, exit process with success status
			fclose(fp);
	       	_exit(0);
	  	}	
	}

  	else
  	{
        // Close the file resposible for writing in pipe with child
        close(fd1[1]);

        // Wait for child to terminate
        waitpid(childpid1, &returnStatus_1, 0);

        // Pointer and tmps for reading 
        FILE* fp = fdopen(fd1[0], "r");
        int lines = 0;
        char ch;

        // Count the lies
		while(!feof(fp))
		{
		  ch = fgetc(fp);
		  if(ch == '\n')
		  {
		    lines++;
		  }
		}

		// Display the result and close pointer
		printf("%d\n", lines);
		fclose(fp);
  	}	

  	// Successfully end program
	return 0;

}

// Print file mode into the given char
void getMode(char *out, struct stat statbuf){

		if(S_ISBLK(statbuf.st_mode))
		    *out = 'b';
		else if(S_ISCHR(statbuf.st_mode))
		    *out = 'c';
		else if(S_ISDIR(statbuf.st_mode))
		    *out = 'd';
		else if(S_ISFIFO(statbuf.st_mode))
		    *out = 'p';
		else if(S_ISREG(statbuf.st_mode))
		    *out = '-';
		else if(S_ISLNK(statbuf.st_mode))
		    *out = 'l';
		else if(S_ISSOCK(statbuf.st_mode))
		    *out = 's';
}
