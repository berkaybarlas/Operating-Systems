#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if(argc < 3) {
		printf("\n3 Command Line Argument Needed!");
		exit(-1); 
	} 
    char * consumerProcuderProgramName = argv[1];
    
    int numProcesses = atoi(argv[2]);
    char message[] = "The OS assignment deadline is approaching.";

    if(argc > 3)
        strcpy(message, argv[3]);

    printf("Parent: Playing Chinese whisper with %d processes.\n",numProcesses);
    
    if( numProcesses < 2) {
        fprintf(stderr,"Children Process number can not less than 2.");
        exit(-1);
    }
    pid_t pid;
	
    char path[100];// = "./";
    char path1[] = "p3_consumer_producer.c";
    snprintf(path, sizeof path, "%s%s", "./", consumerProcuderProgramName);

    pid = fork();
    if (pid == 0) { /* child process */
		char *argv[] = { "gcc", "-o", "./whisperer", path, "-lrt", 0 };
        execvp(argv[0], argv);
	} else if (pid > 0) { /* parent process */
		wait(NULL);
		//printf("Child finished compiling. Pid: %d .NumProcesses: %d\n", pid, numProcesses);
        int i = 0;
        while(i < numProcesses && pid > 0) {
            i++;
            
            pid = fork();
            if (pid == 0) { /* child process */
                char mode = '0';
                if(i == 1) mode = '1' ;
                if(i == numProcesses) mode = '2' ; 
                char index[10];
                sprintf(index, "%d", i);
                char *argv[] = {"./whisperer", message, &mode, index, 0};
                execvp(argv[0], argv);
            } else {
                wait(NULL);
            }
        }   
	}
    if (pid > 0) {
        printf("Parent terminating...\n");
    }
    return 0;
}
