/*
 * shelldon interface program

KUSIS ID: 60210 PARTNER NAME: Ege Onat Özsüer
KUSIS ID: 54512 PARTNER NAME: Berkay Barlas

 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>  // directory lib

#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */

int parseCommand(char inputBuffer[], char *args[],int *background);
void checkRedirection(char *args[], int *redirect, char outFile[]);
int codesearch(char dir[], char *args[]);
int findInFile(char dir[], char keyword[]);
int oneMinSong(char *args[]);

int main(void)
{
  char inputBuffer[MAX_LINE]; 	        /* buffer to hold the command entered */
  int background;             	        /* equals 1 if a command is followed by '&' */
 	int redirect;											/* equals 1 if redirect with trunc, 2 if append*/
 	char outFile[MAX_LINE];						/* 80 chars to hold the output file with path.*/
  char *args[MAX_LINE/2 + 1];	        /* command line (of 80) has max of 40 arguments */
  pid_t child;            		/* process id of the child process */
  int status;           		/* result from execv system call*/
  int shouldrun = 1;
	
  int i, upper;
		
  while (shouldrun){            		/* Program terminates normally inside setup */
    background = 0;
		
    shouldrun = parseCommand(inputBuffer,args,&background);       /* get next command */
				
    if (strncmp(inputBuffer, "exit", 4) == 0)
    {
      shouldrun = 0;     /* Exiting from shelldon*/
    }
    
    if (shouldrun) {
      /*
			After reading user input, the steps are 
			(1) Fork a child process using fork()
			(2) the child process will invoke execv()
			(3) if command included &, parent will invoke wait()
       */

	    child = fork();
			if(child == 0) 
			{
				oneMinSong(args);
				return 0;
			}
			else if(background == 0)
			{
				int childStatus;
				waitpid(child, &childStatus, 0);
			}
  	}
  }
  wait(NULL);
  return 0;
}

void checkRedirection(char *args[], int *redirect, char outFile[])
{
	int i=0;
  while(args[i] != NULL)
  {
  	if(strcmp(args[i], ">") == 0)
  	{
  		*redirect = 1;
  		strcpy(outFile, args[i+1]);
  		args[i] = NULL;
  		break;
  	}
  	else if(strcmp(args[i], ">>") == 0)
  	{
  		*redirect = 2;
  		strcpy(outFile, args[i+1]);
  		args[i] = NULL;
  		break;
  	}
  	i++;
	}
}


// crontab 15 01git o * * *
int oneMinSong(char *args[]) 
{
	int hour = 0;
	int min = 0;

	if(args[2] == NULL) {
		printf("Error: Please specify the song file!\n");
		return -1;
	}

	char delim[] = ".";

	char *ptr = strtok(args[1], delim);
	if(ptr != NULL) {
		hour = atoi(ptr);
		min =	atoi(strtok(NULL, delim));
	}

	FILE* file_ptr = fopen("temp", "w");
	fprintf(file_ptr, "%d %d * * * /usr/bin/mpg123 -q %s\n", min, hour, args[2]);
	fprintf(file_ptr, "%d %d * * * pkill mpg123\n", min+1, hour);
  fclose(file_ptr);
	char *cronArgs[2];
	strcpy(cronArgs[0], "crontab");
	strcpy(cronArgs[1], "./temp");
	execvp(cronArgs[0], cronArgs);
	return 0;
//(crontab -l && echo "1 1  * * *  test") | crontab -
//28 16 * * * /usr/bin/mpg123 -q /home/berkay/Desktop/test.mp329 
//16 * * * pkill mpg123
}

/** 
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */

int parseCommand(char inputBuffer[], char *args[],int *background)
{
    int length,		/* # of characters in the command line */
      i,		/* loop index for accessing inputBuffer array */
      start,		/* index where beginning of next command parameter is */
      ct,	        /* index of where to place the next parameter into args[] */
      command_number;	/* index of requested command number */
    
    ct = 0;
	
    /* read what the user enters on the command line */
    do {
	  printf("shelldon>");
	  fflush(stdout);
	  length = read(STDIN_FILENO,inputBuffer,MAX_LINE); 
    }
    while (inputBuffer[0] == '\n'); /* swallow newline characters */
	
    /**
     *  0 is the system predefined file descriptor for stdin (standard input),
     *  which is the user's screen in this case. inputBuffer by itself is the
     *  same as &inputBuffer[0], i.e. the starting address of where to store
     *  the command that is read, and length holds the number of characters
     *  read in. inputBuffer is not a null terminated C-string. 
     */    
    start = -1;
    if (length == 0)
      exit(0);            /* ^d was entered, end of user command stream */
    
    /** 
     * the <control><d> signal interrupted the read system call 
     * if the process is in the read() system call, read returns -1
     * However, if this occurs, errno is set to EINTR. We can check this  value
     * and disregard the -1 value 
     */

    if ( (length < 0) && (errno != EINTR) ) {
      perror("error reading the command");
      exit(-1);           /* terminate with error code of -1 */
    }
    
    /**
     * Parse the contents of inputBuffer
     */
    
    for (i=0;i<length;i++) { 
      /* examine every character in the inputBuffer */
      
      switch (inputBuffer[i]){
      case ' ':
      case '\t' :               /* argument separators */
	if(start != -1){
	  args[ct] = &inputBuffer[start];    /* set up pointer */
	  ct++;
	}
	inputBuffer[i] = '\0'; /* add a null char; make a C string */
	start = -1;
	break;
	
      case '\n':                 /* should be the final char examined */
	if (start != -1){
	  args[ct] = &inputBuffer[start];     
	  ct++;
	}
	inputBuffer[i] = '\0';
	args[ct] = NULL; /* no more arguments to this command */
	break;
	
      default :             /* some other character */
	if (start == -1)
	  start = i;
	if (inputBuffer[i] == '&') {
	  *background  = 1;
	  inputBuffer[i-1] = '\0';
	}
      } /* end of switch */
    }    /* end of for */
    
    /**
     * If we get &, don't enter it in the args array
     */
    
    if (*background)
      args[--ct] = NULL;
    
    args[ct] = NULL; /* just in case the input line was > 80 */
    
    return 1;
    
} /* end of parseCommand routine */

