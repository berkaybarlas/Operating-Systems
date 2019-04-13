#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


void partB() {
	pid_t pid;
	
	pid = fork();
	if (pid == 0) { /* child process */
		char *cmd[] = {"/bin/ps", "f", 0};         		
		execv(cmd[0], cmd);
	} else if (pid > 0) { /* parent process */
		wait(NULL);
		printf("Child finished execution\n");
		}
}

int main()
{
	partB();
	return 0;
}
