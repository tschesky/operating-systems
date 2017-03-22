#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
// Function to print the dir - takes name of the directory as parameter, depth is only used for printing purposes
void printdir(char *dir, int depth, char* perm, char* tmp)
{
	// Directory pointer
    DIR *dPointer;
    struct dirent *entry;
    struct stat statbuf;

    // Hadling errors for opening the directory
    if((dPointer = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }

    // Set the directory as current working directory
    chdir(dir);
    // Reading files in directory, passing them to entry
    while((entry = readdir(dPointer)) != NULL) {
    	// Putting info into statbuf
        stat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            
            //Ingoring . and ..
            if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0) continue;
            
            printf("%*s%s/\n",depth,"",entry->d_name);
            // Rekursywnie kontynuuj
            printdir(entry->d_name,depth+4, perm, tmp);
        }
        if(!S_ISDIR(statbuf.st_mode)){
		    tmp[0] = ((statbuf.st_mode & S_IRUSR) ? 'r' : '-');
		    tmp[1] = ((statbuf.st_mode & S_IWUSR) ? 'w' : '-');
		    tmp[2] = ((statbuf.st_mode & S_IXUSR) ? 'x' : '-');
		    tmp[3] = ((statbuf.st_mode & S_IRGRP) ? 'r' : '-');
		    tmp[4] = ((statbuf.st_mode & S_IWGRP) ? 'w' : '-');
		    tmp[5] = ((statbuf.st_mode & S_IXGRP) ? 'x' : '-');
		    tmp[6] = ((statbuf.st_mode & S_IROTH) ? 'r' : '-');
		    tmp[7] = ((statbuf.st_mode & S_IWOTH) ? 'w' : '-');
		    tmp[8] = ((statbuf.st_mode & S_IXOTH) ? 'x' : '-');
		}        
		if(strcmp(tmp, perm) == 0){
			time_t t = statbuf.st_mtime;
			struct tm* lt;
			lt = localtime(&t);
			char timbuf[80];
			strftime(timbuf, sizeof(timbuf), "%c", lt);
			printf("%*s%s\t\t\tPermissions: %s, Size: %d bytes, Last modified: %s\n",depth,"",entry->d_name, tmp, (int)statbuf.st_size, timbuf);

		}
    }
    chdir("..");
    closedir(dPointer);
}

int main(int argc, char* argv[]){
	
	if(argc != 3){
		printf("Wrong number of arguments\n");
		return 1;
	}

	char* name = argv[1];
	char* perm = argv[2];
	char tmp[9];
	printdir(name,0, perm, tmp);
	return 0;
}
