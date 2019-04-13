#include <stdlib.h>
#include <sys/types.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <time.h>

#define BUFFER_SIZE 1
#define READ_END	0
#define WRITE_END	1

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int distributeAmongChildren(int childrenNum, int integerNum, int testMode) {
    
    printf("Parent: N = %d, M = %d\n", childrenNum, integerNum);

    if(integerNum < childrenNum || childrenNum < 1) {
        //error message
        fprintf(stderr,"Integer number can not less than Children number.");
        exit(-1);
    }
    
    //create pipeline
    int pipeline[2];
    if(pipe(pipeline) == -1) {
        fprintf(stderr,"Pipe failed\n");
        return 0;
    }

    int rPipeline[2];
    if(pipe(rPipeline) == -1) {
        fprintf(stderr,"Pipe failed\n");
        return 0;
    }

    //** randomly create list **//
    int list[integerNum];
    int l;
    srand(time(NULL));
    for(l = 0; l < integerNum; l++) {
        list[l] = rand() % 101 ;
    }

    //Calculate number of elements to send each child 
    int n = integerNum / childrenNum;
    if(integerNum % childrenNum != 0) {
        n++;
    }

    int i = 0;
    pid_t pid = 1;

    //create childrenNum process
    while(i < childrenNum && pid > 0) {
        i++;
        printf("Sending sublist to process %d\n",i);
        pid = fork(); 
        int j;
        if(pid != 0) {
            int sublistSize = MIN(integerNum - (n - 1) * i, n);
            write(rPipeline[WRITE_END], &sublistSize, sizeof(sublistSize));

            for(j = (i-1)*n; j < n * i && j < integerNum; j++) {
                write(rPipeline[WRITE_END], &list[j], sizeof(list[j]));
                if(testMode)
                    fprintf(stderr,"--index: %d\n", j);
            }
        }
        if(testMode)
            printf("Fork number: %d Pid: %d\n",i, pid);
    }

    if(pid == 0) {
        close(pipeline[READ_END]);
        close(rPipeline[WRITE_END]);

        int integerNum;
        read(rPipeline[READ_END], &integerNum , sizeof(integerNum));
        if(testMode)
            printf("\tChild: %d . Recieved %d sublist elements.\n",i ,integerNum);
        
        //find local max
        int max = -1;
        int j;
        int listElement;
        for(j = 0; j < integerNum; j++) {
            read(rPipeline[READ_END], &listElement , sizeof(listElement));
            if(testMode)
                fprintf(stderr,"--index: %d\n", listElement);
            if(listElement > max) 
                max = listElement;
        }

        write(pipeline[WRITE_END], &max, sizeof(max));
        close(pipeline[WRITE_END]);
        return 1;
    } else if (pid > 0) { /* Parent process for collecting from child*/
        close(pipeline[WRITE_END]);
        close(rPipeline[READ_END]);

        int maxLocal;
        int k;
        int maxGlobal = -1;
        // Find global max
        for(k = 0; k < childrenNum; k++) {
            read(pipeline[READ_END], &maxLocal , sizeof(maxLocal));
            fprintf(stderr,"Max value in Process %d is %d\n",k + 1 , maxLocal);
            if(maxLocal > maxGlobal) {
                maxGlobal = maxLocal;
            }

            if(testMode) {
                printf("\tParent: Recieved %d.\n",maxLocal);
                printf("\tParent: maxGlobal %d.\n",maxGlobal);
            }
            
        }
        //print in standard output
        fprintf(stderr,"Global Maximum is %d \n", maxGlobal);
        close(pipeline[READ_END]);
        close(rPipeline[WRITE_END]);
    }
    
    return 1;
}

int main(int argc, char *argv[]) 
{
    int childrenNum;
    int integerNum;
    int testMode = 0;
    
    if(argc == 1) 
    {
        printf("\n2 Command Line Argument Needed!"); 
    }
    else if(argc >= 3) 
    { 
        integerNum = atoi(argv[1]);
        childrenNum = atoi(argv[2]);
        if(argc >= 4)    
            testMode = atoi(argv[3]); 
        distributeAmongChildren(childrenNum, integerNum, testMode);
    } 

    return 0;
} 