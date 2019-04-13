#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void processCreator() {
	pid_t pid;
	pid_t parent;
	int level = 0;
	parent = getpid();
	int i;

	printf("Base Process ID: %d, level: %d\n", getpid(), level++);

	for(i = 0; i < 4; i++) {
		pid = fork();

		if (pid == 0) { /* child process */
			printf("Process ID: %d, Parent ID: %d, level: %d \n", getpid(), parent, level++);
			parent = getpid();
		}
		else if (pid > 0) { /* parent process */
			wait(NULL);	
		}
	}
}

int main()
{
	processCreator();
	return 0;
}
