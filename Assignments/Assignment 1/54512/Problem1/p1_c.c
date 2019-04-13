#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void zombie() {
	pid_t pid;
	pid = fork();
	if (pid == 0) { /* child process */
		exit(0);
	}
	else if (pid > 0) { /* parent process */
		printf("Zombie process created. \n");
		sleep(5);
		printf("Zombie process killed. \n");
		wait(NULL);
	}
	return;
}

int main()
{
	zombie();
	return 0;
}
