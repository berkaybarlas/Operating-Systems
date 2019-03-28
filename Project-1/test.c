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
				codesearch(".", args);
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

/**
* The codesearch command The command takes an input keyword and 
* scans all the files in the current directory to match the keyword and 
* then it returns the filenames, line numbers where the keyword occurs and 
* finally the line itself
* ex: 92: ./foo.c -> foo(a,b);
*/
int codesearch(char dir[], char *args[] )
{
// if it's recursive call codesearch for sub directories -r
// if it's targeted search only for that file -f -> call findInFile

// Data Type: struct dirent
struct dirent *de;  // Pointer for directory entry  
int recursive = 0;
int targeted = 0;
char keyword[MAX_LINE];
/**
* Should search for " and - 
* str copy according to that 
*/
if(args[2] != NULL)
{
	if(strcmp(args[2], "-r") == 0) 
	{
		recursive = 1;
	} else if(strcmp(args[2], "-f") == 0)
	{
		targeted = 1;
	}
	strcpy(keyword, args[1]);
} else if(args[1] != NULL){
	strcpy(keyword, args[1]);
} else {
	printf("Please specify keyword.\n");
	return -1; 
}

if(!targeted)
{
		DIR *dr = opendir(dir); 
		if (dr == NULL)  // opendir returns NULL if couldn't open directory 
		{ 
				printf("Could not open current directory." ); 
				return -1; 
		} 

		while ((de = readdir(dr)) != NULL)
		{
			char fileName[MAX_LINE];
			strcpy(fileName, de->d_name);
			//printf("%s\n", fileName);
			// type 4 means it's a directory check it's not current or previous one
			if(recursive && de->d_type == 4 && (strcmp(fileName, ".") !=0 && strcmp(fileName, "..") != 0 )) {
				//printf("This a directory %d\n", de->d_type);
				codesearch(fileName, args);
			}
			findInFile(de->d_name, keyword);
		} 
		closedir(dr);
	} else {
		if(args[3] != NULL)
		{
			findInFile(args[3], keyword);
		}	
	}
	return 1;
}

int findInFile(char dir[], char keyword[])
{
	char ch, file_name[25];
	FILE *fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int lineNum  = 0;

	strcpy(file_name, dir);

	fp = fopen(file_name, "r"); // read mode
	if (fp == NULL)
	{
		perror("Error while opening the file.\n");
		return -1;
	}

	// Read file line by line
	// Print if the file contains keyword 
	while ((read = getline(&line, &len, fp)) != -1) 
	{
		lineNum++;
		if(strstr(line, keyword) != NULL )
			printf("%d: %s -> %s", lineNum, dir, line);
	}

	// Alternative way

	// int c;
	// while ((c = getc(fp)) != EOF)
	//         putchar(c);

	fclose(fp);
	if (line)
		free(line);
	return 0;
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

