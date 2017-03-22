#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <ftw.h>


// Function to be passed to ftw()
int parseFile(const char * ftw_filePath,const struct stat * ptr, int flags);
// Global variable, so the function can be able to acces what permissions to look for
char* globalPermission;

int main(int argc, char* argv[]){
	
    // Checking argument count
	if(argc != 3){
		printf("Wrong number of arguments\n");
		return 1;
	}

    // Directory name
	char* name = argv[1];
    // Pointing the global variable to our permissions
	globalPermission = argv[2];

    // Calling ftw()
    ftw(name, parseFile, 1);
	return 0;
}

// Definition of the function that we call with ftw
int parseFile(const char * ftw_filePath,const struct stat * ptr, int flags){
    
    // Buffer for loading permissions of every file
    char permissions[8];
    // Variables for converting time
    time_t t = ptr->st_mtime;
    char timbuf[80];
    struct tm* lt;

    // Actual coversion
    lt = localtime(&t);
    strftime(timbuf, sizeof(timbuf), "%c", lt);
   
    // If the object can't be executed by ftw()
    if(flags == FTW_NS) return 0;

    // If the object is a file
    if(flags == FTW_F){
        permissions[0] = ((ptr->st_mode & S_IRUSR) ? 'r' : '-');
        permissions[1] = ((ptr->st_mode & S_IWUSR) ? 'w' : '-');
        permissions[2] = ((ptr->st_mode & S_IXUSR) ? 'x' : '-');
        permissions[3] = ((ptr->st_mode & S_IRGRP) ? 'r' : '-');
        permissions[4] = ((ptr->st_mode & S_IWGRP) ? 'w' : '-');
        permissions[5] = ((ptr->st_mode & S_IXGRP) ? 'x' : '-');
        permissions[6] = ((ptr->st_mode & S_IROTH) ? 'r' : '-');
        permissions[7] = ((ptr->st_mode & S_IWOTH) ? 'w' : '-');
        permissions[8] = ((ptr->st_mode & S_IXOTH) ? 'x' : '-');
        if(strcmp(permissions, globalPermission)==0){
          printf("%s\t\t\tPermissions: %s, Size: %d bytes, Last modified: %s\n",ftw_filePath, permissions, (int)ptr->st_size, timbuf);
        }

    }
     
    // If the object is a directory, but not . and ..
    if(flags == FTW_D && strcmp(".", ftw_filePath) != 0 && strcmp("..", ftw_filePath) != 0){
        printf("\t\t%s/\n", ftw_filePath);
    }

    return 0;
}