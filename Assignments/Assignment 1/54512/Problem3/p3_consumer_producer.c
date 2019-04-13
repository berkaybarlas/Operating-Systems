#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h> 


void randomSwap(char string[]) {
	//char copy[100];
	//strcpy(copy,string);
	int length = strlen(string);
    int index1 = rand() % length;
	int index2 = rand() % length;
	char temp = string[index1];
    string[index1] = string[index2];
	string[index2] = temp;
}

int main(int argc, char *argv[]) {
	srand(getpid());
	const char *name = "OS";
	const int SIZE = 4096;

	int shm_fd;
	void *ptr;
	int i, index;

	int mode;

	if(argc < 4 ) {
		printf("\n4 Command Line Argument Needed!");
		exit(-1); 
	} 
	char *message = argv[1];
    mode = atoi(argv[2]);
	index = atoi(argv[3]);

	/* open the shared memory segment */
	//if(mode == 1) {
		shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
		/* configure the size of the shared memory segment */
		ftruncate(shm_fd,SIZE);

		/* now map the shared memory segment in the address space of the process */
		ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
//	} 
	// else { 
	// 	shm_fd = shm_open(name, O_RDONLY, 0666);
	// 	/* configure the size of the shared memory segment */
	// 	ftruncate(shm_fd,SIZE);

	// 	/* now map the shared memory segment in the address space of the process */
	// 	ptr = mmap(0,SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
	// }	
	if (shm_fd == -1) {
		perror("shared memory failed\n");
		exit(-1);
	}
	
	/* configure the size of the shared memory segment */
	ftruncate(shm_fd,SIZE);

	/* now map the shared memory segment in the address space of the process */
	
	
	if (ptr == MAP_FAILED) {
		perror("Map failed\n");
		exit(-1);
	}
	char temp[100];
	/* now read from the shared memory region */
	if(mode == 1) {
		strcpy(temp,message);
		sprintf(ptr,"%s",message);
	} else {
		strcpy(temp,ptr);
		randomSwap(temp);
		sprintf(ptr,"%s",temp);
	}
	
	printf("Child %d: %s\n",index ,temp);
	
	/* remove the shared memory segment 
	* Only when last child
	*/
	if(mode == 2) {
		if (shm_unlink(name) == -1) {
			printf("Error removing %s\n",name);
			exit(-1);
		}
	}

	return 0;
}
