#define _GNU_SOURCE  

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

// Waiting for all child processes to end
int wait_for_all(int proc_count);

int main(int argc, char **argv) {

    // We set a flag, depending on whether the -w parameter is used
    int option = 0;

    // Checking argument count
    if(argc > 3){
        printf("Wrong number of arguments!\n");
        fflush(stdout);
        return 1;
    } else if(argc == 3){
        if(strcmp(argv[2], "-w") == 0) option = 1;
    }

    // We read the file path and create a directory pointer
    char* file_path = argv[1];
    DIR* dir;

    // Handling the pointer
    if((dir = opendir(file_path)) == NULL){
        printf("opendir(): %s: %s\n", file_path, strerror(errno));
        return -1;
    }

    // For directory reading
    struct dirent* dir_ent;

    char* tmp_dir_path = malloc(1000);
    int counter = 0, tmp_counter;
    int proc_count = 0;
    pid_t pid;

    // Reading current directory
    while((dir_ent = readdir(dir))){

        // We skip "." and ".." locations
        if(strcmp(dir_ent->d_name,".") && strcmp(dir_ent->d_name,"..")){
            
            // If it's a directory
            if(dir_ent->d_type == DT_DIR){
                // Setting new path
                sprintf(tmp_dir_path, "%s/%s", file_path, dir_ent->d_name);
                // Forking new proccess
                proc_count++;
                pid = fork();
                if(pid == 0){
                    // The childer proccess reccursively starts the program again, for new directory
                    execl("./counter", "counter", tmp_dir_path, argv[2], (char*)0);
                }
            } 
            // If it's not directory - we count it as a file
            else{
                counter += 1;
            }
        }
    }

    // If "-w" was used, sleep
    if(option == 1)
        sleep(15);

    // Count all the childer
    tmp_counter = wait_for_all(proc_count);

    // If -1 we have an error
    if(tmp_counter == -1)
        return -1;

    // Add the given result to global counter
    counter += tmp_counter;     // add results from children

        // Print current dir count
        printf("dir_path: %s\n\tfiles: %d\n\n", file_path, counter);
        fflush(stdout);

    // Close current directory and free the temp. variable
    closedir(dir);
    free(tmp_dir_path);

    // Return the counter so parent proccess can collect it
    return counter;
}

// Function responsible for waiting for all childern and collecting current count from them
int wait_for_all(int proc_count){
    int status_sum = 0;
    int pid;
    int status;
    while(proc_count){
        pid = wait(&status);
        if(WEXITSTATUS(status) == -1)
            return -1;
        status_sum += WEXITSTATUS(status);
        proc_count--;
    }
    return status_sum;
}