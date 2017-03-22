#define _GNU_SOURCE  
#include <sched.h> 

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/wait.h>

// Global counter
int counter;

// Testing functions for every specified fun: fork, vfork, fork-clone, vfork-clone
int forkTest(int n);
int vforkTest(int n);
int cloneForkTest(int n);
int cloneVforkTest(int n);

// Function executed by the clone implementation
int cloneFunction(void* arg);

// Returns a difference between two tms structures in a new struct
struct tms diffTime(struct tms stop, struct tms start);

// No of proccessor ticks per second
double getClktck();

int main(int argc, char* argv[]){

    // Check no. of arguments
    if(argc != 2){
        printf("Wrong number of arguments\n");
        return 1;
    }

    // Read the no. of runs, it's the initial number, later we just increment it by a fixed number to simplify using the program
    int cycles = atoi(argv[1]);

    // Table storing names of the output files
    char* files[4] = {"results/fork.txt","results/vfork.txt","results/forkClone.txt","results/vforkClone.txt"};

    // Temporary variables
    int a, N;

    // Structs for time measurment
    clock_t real;
    struct tms beg, end, result;

    // Buffer for simplified writing to file
    char* line = malloc(300);

    // No of ticks for time measurment
    double TICK = getClktck();

    // First for reponsible for executing every 4 funs
    for(int j = 0 ; j < 4 ; ++j){

        // Opening the file
        FILE* fHandle = fopen(files[j], "wb");
    
        // Handling problems with opening
        if(fHandle == NULL){
            printf("Some trouble opening the file\n");
            if( fclose(fHandle) != 0){
                printf("Some trouble closing the file");
            }
            return 1;
        }

        // Second for repeats the test for different no. of processes
        for(int i = 0 ; i < 4 ; ++i){
            real = times(&beg);
            N = cycles + (15000 * i);

            // Chooses which test to run, tests return the real elapsed time from the child proccess
            switch(j){
                case 0:
                    a = forkTest(N);
                    break;
                case 1:
                    a = vforkTest(N);
                    break;
                case 2:
                    a = cloneForkTest(N);
                    break;
                case 3:
                    a = cloneVforkTest(N);
                    break;
            }

            // Elapsed time from parent proccess
            real = times(&end) - real;
            result = diffTime(end, beg);

            // Writing to buffer, later saving to file
            sprintf(line, "%d %.2f %.2f %.2f %.2f ", N, result.tms_stime/TICK, result.tms_utime/TICK, real/TICK, (result.tms_stime + result.tms_utime)/TICK);
            fwrite(line, strlen(line), 1, fHandle);

            sprintf(line, "%.2f %.2f %.2f %.2f ", result.tms_cstime/TICK, result.tms_cutime/TICK, a/TICK, (result.tms_cstime + result.tms_cutime)/TICK);
            fwrite(line, strlen(line), 1, fHandle);

            sprintf(line, "%.2f %.2f %.2f %.2f\n", result.tms_cstime/TICK + result.tms_stime/TICK, result.tms_cutime/TICK + result.tms_utime/TICK, a/TICK + real/TICK, (result.tms_cstime + result.tms_cutime + result.tms_stime + result.tms_utime)/TICK);
            fwrite(line, strlen(line), 1, fHandle);
        }

        // Closing the file after all has been done
        if( fclose(fHandle) != 0){
            printf("Some trouble closing the file");
        }

    }

    return 0;
}

// Testing fork() - new proccess, copies data
int forkTest(int n){

    pid_t pid;
    int status, allTime;
    struct timeval time, temp;

    allTime = 0;
    counter = 0;


    for(int i = 0 ; i < n ; ++i){
        pid = fork();
        if(pid < 0){                                // error forking
            printf("forkTest(): %d: %s\n", errno, strerror(errno));
        }else if(pid == 0){                         // for child proccess
            gettimeofday(&time, NULL);
            temp.tv_usec = time.tv_usec;
            temp.tv_sec = time.tv_sec;
            counter+=1;
            gettimeofday(&time, NULL);
            // Exit passing the elapsed proccess time
            _exit(time.tv_usec - temp.tv_usec + (time.tv_sec - temp.tv_sec)*1000);
        }else{                                      // for parent proccess
            if(pid!=0 && wait(&status) != pid){     // error in wait()
                printf("wait(): %d: %s\n", errno, strerror(errno));
            }
            if(WIFEXITED(status)){                  // add the returned proccess time to overall time
                allTime += WEXITSTATUS(status);
            }
        }
    }
    printf("Fork test: \t%d\n", counter);
    return allTime;

}

// Testing vfork() - new proccess, shares data
int vforkTest(int n){

    pid_t pid;
    int status, allTime;
    struct timeval time, temp;

    counter = 0;
    allTime = 0;

    for(int i = 0 ; i < n ; ++i){
        pid = vfork();
        if(pid < 0){
            printf("forkTest(): %d: %s\n", errno, strerror(errno));
        }else if(pid == 0){
            gettimeofday(&time, NULL);
            temp.tv_usec = time.tv_usec;
            temp.tv_sec = time.tv_sec;
            counter+=1;
            gettimeofday(&time, NULL);
            _exit(time.tv_usec - temp.tv_usec + (time.tv_sec - temp.tv_sec)*1000);
        }else{
            if(pid!=0 && wait(&status) != pid){
                printf("wait(): %d: %s\n", errno, strerror(errno));
            }
            if(WIFEXITED(status)){
                allTime += WEXITSTATUS(status);
            }
        }
    }
    printf("Vfork test: \t%d\n", counter);
    return allTime;
}

// Function used by the clone implementations
int cloneFunction(void* arg){
    struct timeval time, temp;

    gettimeofday(&time, NULL);
    temp.tv_usec = time.tv_usec;
    temp.tv_sec = time.tv_sec;
    counter+=1;
    gettimeofday(&time, NULL);
    _exit(time.tv_usec - temp.tv_usec + (time.tv_sec - temp.tv_sec)*1000);
}

// Close used to simulate fork()
int cloneForkTest(int n){
    pid_t pid;
    int i;
    int status;
    counter = 0;
    int allTime;
    allTime = 0;
    void* child_stack = malloc(10000);
    child_stack += 10000;

    for(i=0 ; i<n ; ++i){
        if((pid = clone(&cloneFunction, child_stack, SIGCHLD, NULL)) < 0){
            printf("cloneForkTest(): %d: %s\n", errno, strerror(errno));
        }
        if(pid!=0 && wait(&status) != pid){
            printf("wait(): %d: %s\n", errno, strerror(errno));
        }
        if(WIFEXITED(status)){
            allTime += WEXITSTATUS(status);
        }
    }
    printf("Clone fork test: \t%d\n", counter);
    return allTime;
}

// Clone used to simulate vfork()
int cloneVforkTest(int n){
    pid_t pid;
    int i;
    int status;
    counter = 0;
    int allTime;
    allTime = 0;
    void* child_stack = NULL;
    child_stack = malloc(10000);
    child_stack += 10000;

    for(i=0 ; i<n ; ++i){
        if((pid = clone(&cloneFunction, child_stack, SIGCHLD | CLONE_VM | CLONE_VFORK, NULL)) < 0){
            printf("cloneVforkTest(): %d: %s\n", errno, strerror(errno));
        }
        if(pid!=0 && wait(&status) != pid){
            printf("wait(): %d: %s\n", errno, strerror(errno));
        }
        if(WIFEXITED(status)){
            allTime += WEXITSTATUS(status);
        }
    }
    printf("clone vfork test: \t%d\n", counter);
    return allTime;
}

// Time difference returned in a tms struct
struct tms diffTime(struct tms stop, struct tms start){
    struct tms result;
    result.tms_stime = stop.tms_stime - start.tms_stime;
    result.tms_utime = stop.tms_utime - start.tms_utime;
    result.tms_cstime = stop.tms_cstime - start.tms_cstime;
    result.tms_cutime = stop.tms_cutime - start.tms_cutime;
    return result;
}

// Returns the no of clicks per sec, casted to double
double getClktck(){
    return (double)sysconf(_SC_CLK_TCK);
}


