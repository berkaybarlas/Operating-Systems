/*
 * shelldon interface program
COMP 304 Spring 2019 Project-1
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
#include <dirent.h> // directory lib
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 80    /* 80 chars per line, per command, should be enough. */
#define HIST_LENGTH 10 // Number of args to be stored in history
#define MODULE "oldestChild"
//int parseCommand(char inputBuffer[], char *args[],int *background);

int codesearch(char dir[], char *args[]);
int findInFile(char dir[], char keyword[]);
int oneMinSong(char *args[]);
int parseCommand(char inputBuffer[], char *args[], int *argct, int *background);
int redirect(char *args[], char outFile[]);
void saveHistory(char inputBuffer[], char *args[], char *argsHistory[][MAX_LINE / 2 + 1],
                 char buffersHistory[][MAX_LINE], int argct, int background);
void printHistory(int counter, char *argsHistory[][MAX_LINE / 2 + 1]);
void executeFromHistory(int n, char *args[], char *argsHistory[][MAX_LINE / 2 + 1],
                        int *background);
int oldestChild(char *args[], int *previousPid);
void closeShelldon();

int main(void)
{
  char inputBuffer[MAX_LINE];                       /* buffer to hold the command entered */
  int background;                                   /* equals 1 if a command is followed by '&' */
  char outFile[MAX_LINE];                           /* 80 chars to hold the output file with path.*/
  char *args[MAX_LINE / 2 + 1];                     /* command line (of 80) has max of 40 arguments */
  char *argsHistory[HIST_LENGTH][MAX_LINE / 2 + 1]; // Saves ptrs to args in buffersHistory
  char buffersHistory[HIST_LENGTH][MAX_LINE];       // Saves copies of inputBuffers
  int counter = 0;                                  // Keeps track of lines of input taken so far
  pid_t child;                                      /* process id of the child process */
  int status;                                       /* result from execv system call*/
  int shouldrun = 1;
  int previousPid = 10;

  while (shouldrun)
  { /* Program terminates normally inside setup */
    background = 0;
    int argct = 0; // number of separate arguments in current args, including NULL

    bool tempFilled = false;
    char *temp[MAX_LINE / 2 + 1]; //Temp args holder to help save !X commands

    /* get next command */
    shouldrun = parseCommand(inputBuffer, args, &argct, &background);

    if (strncmp(inputBuffer, "exit", 4) == 0)
    {
      shouldrun = 0; /* Exiting from shelldon*/
      closeShelldon();
    }

    if (shouldrun)
    {
      /*
			After reading user input, the steps are 
			(1) Fork a child process using fork()
			(2) the child process will invoke execv()
			(3) if command included &, parent will invoke wait()
       */

      counter++;

      if (strcmp(args[0], "!!") == 0)
      {
        if (counter == 1)
        {
          printf("No commands in history.\n");
          continue;
        }
        memcpy(temp, args, MAX_LINE / 2 + 1);
        tempFilled = true;
        executeFromHistory(0, args, argsHistory, &background);
      }
      else if (args[0][0] == '!')
      {
        char *end;
        long l = strtol(args[0] + 1, &end, 10);
        if (end == args[0] + 1 || *end != '\0')
        {
          printf("Invalid input after \"!\"\n");
          continue;
        }
        else
        {
          int historyInd = counter - l - 1;
          int savedArgCount = (counter - 1 < HIST_LENGTH) ? counter - 1 : HIST_LENGTH;
          if (historyInd >= savedArgCount || historyInd < 0)
          {
            printf("No such command in history\n");
            continue;
          }
          memcpy(temp, args, MAX_LINE / 2 + 1);
          tempFilled = true;
          executeFromHistory(historyInd, args, argsHistory, &background);
        }
      }
      if (strcmp(args[0], "history") == 0)
      {
        printHistory(counter, argsHistory);
        saveHistory(inputBuffer, args, argsHistory, buffersHistory, argct, background);
        continue;
      }
      
      if (strcmp(args[0], "codesearch") == 0)
      {
        codesearch(".", args);
      }
      else if (strcmp(args[0], "oldestchild") == 0)
      {
        oldestChild(args, &previousPid);
      }
      else if (strcmp(args[0], "birdakika") == 0)
      {
        oneMinSong(args);
      } else {
        child = fork();
        if (child == 0)
        {
        
        redirect(args, outFile); //checking and doing redirection
        
        if (args[0] != NULL && strncmp(args[0], "./", 2) != 0)
        {  
          char path[MAX_LINE] = "/bin/";
          strcat(path, args[0]);
          status = execv(path, args);
        } else {
          status = execv(args[0], args);
        }
        if (status < 0 )
          printf("Failed to find executable\n");
        return 0;
        }
      }
      
      // only parent execute this part
      if (background == 0)
      {
        int childStatus;
        waitpid(child, &childStatus, 0);
      }
      //Save args pointers and input buffer into history
      if (tempFilled)
      {
        memcpy(args, temp, MAX_LINE / 2 + 1);
        background = 0;
      }
      saveHistory(inputBuffer, args, argsHistory, buffersHistory, argct, background);
    }
    waitpid(-1, NULL, WNOHANG);
  }
  wait(NULL);
  return 0;
}

void closeShelldon() {
  pid_t child;
  child = fork();
  int childStatus;
  if(child == 0) {
    char *rmModArgs[] = {
      "/usr/bin/sudo",
      "rmmod",
      MODULE,
      0
    };
   execv(rmModArgs[0], rmModArgs);
  } else {
    printf("Good bye...\n");
  }
}

void executeFromHistory(int n, char *args[], char *argsHistory[][MAX_LINE / 2 + 1],
                        int *background)
{
  memcpy(args, argsHistory[n], MAX_LINE / 2 + 1);
  for (int i = 0; i < MAX_LINE / 2 + 1; i++)
  {
    if (args[i] != NULL)
    {
      if (strcmp(args[i], "&") == 0)
      {
        args[i] = NULL;
        *background = 1;
      }
    }
    else
    {
      break;
    }
  }
}

void printHistory(int counter, char *argsHistory[][MAX_LINE / 2 + 1])
{
  int linesToPrint = (counter - 1 < HIST_LENGTH) ? counter - 1 : HIST_LENGTH;
  for (int i = 0; i < linesToPrint; i++)
  {
    printf("%d ", counter - i - 1);
    for (int j = 0; j < MAX_LINE / 2 + 1; j++)
    {
      if (argsHistory[i][j] == NULL)
      {
        printf("\n");
        break;
      }
      else
      {
        printf("%s ", argsHistory[i][j]);
      }
    }
  }
}

//Saves history
void saveHistory(char inputBuffer[], char *args[], char *argsHistory[][MAX_LINE / 2 + 1],
                 char buffersHistory[][MAX_LINE], int argct, int background)
{
  if (background == 1)
  {
    char *bgArg = args[argct - 2] + strlen(args[argct - 2]) + 1;
    args[argct - 1] = bgArg;
    args[argct] = NULL;
    argct += 1;
  }
  for (int i = HIST_LENGTH - 1; i > 0; i--)
  {
    memcpy(buffersHistory[i], buffersHistory[i - 1], MAX_LINE);
    memcpy(argsHistory[i], argsHistory[i - 1], MAX_LINE / 2 + 1);
    for (int j = 0; j < MAX_LINE / 2 + 1; j++)
    {
      if (argsHistory[i][j] != NULL)
      {
        argsHistory[i][j] += buffersHistory[i] - buffersHistory[i - 1];
      }
    }
  }
  memcpy(buffersHistory[0], inputBuffer, MAX_LINE);
  argsHistory[0][0] = buffersHistory[0];
  for (int i = 1; i < MAX_LINE / 2 + 1; i++)
  {
    if (args[i] != NULL)
    {
      int ptrdif = args[i] - args[0];
      argsHistory[0][i] = argsHistory[0][0] + ptrdif;
    }
    else
    {
      argsHistory[0][i] = NULL;
    }
  }
}

//Does redirection if user requested it
int redirect(char *args[], char outFile[])
{
  int redirect = 0;
  int i = 0;
  while (args[i] != NULL)
  {
    if (strcmp(args[i], ">") == 0)
    {
      redirect = 1;
      strcpy(outFile, args[i + 1]);
      args[i] = NULL;
      break;
    }
    else if (strcmp(args[i], ">>") == 0)
    {
      redirect = 2;
      strcpy(outFile, args[i + 1]);
      args[i] = NULL;
      break;
    }
    i++;
  }
  if (redirect != 0)
  {
    int fd;
    if (redirect == 1)
    {
      fd = open(outFile, O_CREAT | O_RDWR | O_TRUNC, 0644);
    }
    else if (redirect == 2)
    {
      fd = open(outFile, O_CREAT | O_RDWR | O_APPEND, 0644);
    }
    if (fd < 0)
    {
      printf("Failed to create output file");
      exit(1);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
  }
  return 0;
}

/**
* The codesearch command The command takes an input keyword and 
* scans all the files in the current directory to match the keyword and 
* then it returns the filenames, line numbers where the keyword occurs and 
* finally the line itself
* ex: 92: ./foo.c -> foo(a,b);
*/
int codesearch(char dir[], char *args[])
{
  // if it's recursive call codesearch for sub directories -r
  // if it's targeted search only for that file -f -> call findInFile

  // Data Type: struct dirent
  struct dirent *de; // Pointer for directory entry
  int recursive = 0;
  int targeted = 0;
  char keyword[MAX_LINE];
  /**
* Should search for " and - 
* str copy according to that 
*/
  if (args[2] != NULL)
  {
    if(strstr(args[2],"-") == NULL) 
    {
        if (strcmp(args[2], "-r") == 0)
      {
        recursive = 1;
      }
      else if (strcmp(args[2], "-f") == 0)
      {
        targeted = 1;
      }
      strcpy(keyword, args[1]);
    } 
    else
    if(strstr(args[1],"-") == NULL) 
    {
        if (strcmp(args[1], "-r") == 0)
      {
        recursive = 1;
      }
      else if (strcmp(args[1], "-f") == 0)
      {
        targeted = 1;
      }
      strcpy(keyword, args[2]);
    }  
  }
  else if (args[1] != NULL)
  {
    strcpy(keyword, args[1]);
  }
  else
  {
    printf("Please specify keyword.\n");
    return -1;
  }

  if(strstr(keyword,"\"") == NULL) {
    printf("Please specify keyword.\n");
    return -1;
  }
printf(" keyword:%s\n",keyword);
  // remove quotes
  int i = 0;
	int currentIndex = 0;
	while(keyword[i] != '\0') {
	  if(keyword[i] != '\"') {
	  	keyword[currentIndex++] = keyword[i];
	  }
	  i++;
	}
	keyword[currentIndex] = '\0';

  if (!targeted)
  {
    DIR *dr = opendir(dir);
    if (dr == NULL) // opendir returns NULL if couldn't open directory
    {
      printf("Could not open current directory.\n");
      return -1;
    }

    while ((de = readdir(dr)) != NULL)
    {
      char fileName[MAX_LINE];
      strcpy(fileName, de->d_name);
      //printf("%s\n", fileName);
      // type 4 means it's a directory check it's not current or previous one
      if (recursive && de->d_type == 4 && (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0))
      {
        //printf("This a directory %d\n", de->d_type);
        codesearch(fileName, args);
      }
      findInFile(de->d_name, keyword);
    }
    closedir(dr);
  }
  else
  {
    if (args[3] != NULL)
    {
      findInFile(args[3], keyword);
    } else {
      printf("Please specify target file!\n");
    }
  }
  return 1;
}

int findInFile(char dir[], char keyword[])
{
  char ch, file_name[25];
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int lineNum = 0;

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
    if (strstr(line, keyword) != NULL)
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

// crontab 15 01git o * * *
//28 16 * * * /usr/bin/mpg123 -q /home/berkay/Desktop/test.mp329
//16 * * * pkill mpg123
int oneMinSong(char *args[])
{
  int hour = 0;
  int min = 0;
  pid_t child;

  if (args[2] == NULL)
  {
    printf("Error: Please specify the song file!\n");
    return -1;
  }

  char delim[] = ".";

  char *ptr = strtok(args[1], delim);
  if (ptr != NULL)
  {
    hour = atoi(ptr);
    min = atoi(strtok(NULL, delim));
  }
  if(strstr(args[2],"/") == NULL) 
  {
    char buf[MAX_LINE];
    getcwd(buf,1024);
    char temp[MAX_LINE];
    strcpy(temp,buf);
    strcat(temp,"/");
    strcat(temp,args[2]);
    strcpy(args[2],temp);
  }
  FILE *file_ptr = fopen("temp", "w");
  fprintf(file_ptr, "%d %d * * * /usr/bin/mpg123 -q %s\n", min, hour, args[2]);
  fprintf(file_ptr, "%d %d * * * pkill mpg123\n", min + 1, hour);
  fclose(file_ptr);
  // Create a child and call crontab 
  int childStatus;
  child = fork();
  
  if(child == 0) {
    char *cronArgs[] = {
      "/usr/bin/crontab",
      "temp",
      0
    };
    execv(cronArgs[0], cronArgs);
  } else {
    waitpid(child, &childStatus, 0);
  }
  remove("temp"); //remove temp
  //printf("Removed file %d\n",childStatus);
  return 0;
}

int oldestChild(char *args[], int *previousPid) {
  pid_t child;
  int pid;
  
  
  if(args[1] == NULL) {
    printf("Please indicate processID!\n");
    return -1;
  }
  pid = atoi(args[1]);
  if(*previousPid == pid) {
    printf("Please indicate processID!\n");
    return -1;
  }
  *previousPid = pid;
  

  child = fork();
  int childStatus;
  if(child == 0) {
    char *rmModArgs[] = {
      "/usr/bin/sudo",
      "rmmod",
      MODULE,
      0
    };
    execv(rmModArgs[0], rmModArgs);
  } else {
    waitpid(child, &childStatus, 0);
    if(childStatus < 0)
      printf("Error during rmmode !\n");
    
    char processID[32];
    sprintf(processID, "processID=%d", pid);
    char *insModArgs[] = {
      "/usr/bin/sudo",
      "insmod",
      "oldestChild.ko",
      processID,
      0
    };
    child = fork();
    int childStatus;
    if(child == 0) {
      execv(insModArgs[0], insModArgs);
      printf("oldestchild module installed with parameter %s\n",processID);
    } else {
      waitpid(child, &childStatus, 0);
      if (childStatus < 0) {
        printf("Error during insmod\n");
      }
    }
  }
  return 0;
}

/** 
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */

int parseCommand(char inputBuffer[], char *args[], int *argct, int *background)
{
  int length,         /* # of characters in the command line */
      i,              /* loop index for accessing inputBuffer array */
      start,          /* index where beginning of next command parameter is */
      ct,             /* index of where to place the next parameter into args[] */
      command_number; /* index of requested command number */

  ct = 0;

  /* read what the user enters on the command line */
  do
  {
    printf("shelldon>");
    fflush(stdout);
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
  } while (inputBuffer[0] == '\n'); /* swallow newline characters */

  /**
	 *  0 is the system predefined file descriptor for stdin (standard input),
	 *  which is the user's screen in this case. inputBuffer by itself is the
	 *  same as &inputBuffer[0], i.e. the starting address of where to store
	 *  the command that is read, and length holds the number of characters
	 *  read in. inputBuffer is not a null terminated C-string. 
	 */
  start = -1;
  if (length == 0)
    exit(0); /* ^d was entered, end of user command stream */

  /** 
	 * the <control><d> signal interrupted the read system call 
	 * if the process is in the read() system call, read returns -1
	 * However, if this occurs, errno is set to EINTR. We can check this  value
	 * and disregard the -1 value 
	 */

  if ((length < 0) && (errno != EINTR))
  {
    perror("error reading the command");
    exit(-1); /* terminate with error code of -1 */
  }

  /**
	 * Parse the contents of inputBuffer
	 */

  for (i = 0; i < length; i++)
  {
    /* examine every character in the inputBuffer */
    switch (inputBuffer[i])
    {
    case ' ':
    case '\t': /* argument separators */
      if (start != -1)
      {
        args[ct] = &inputBuffer[start]; /* set up pointer */
        ct++;
      }
      inputBuffer[i] = '\0'; /* add a null char; make a C string */
      start = -1;
      break;
    case '\n': /* should be the final char examined */
      if (start != -1)
      {
        args[ct] = &inputBuffer[start];
        ct++;
      }
      inputBuffer[i] = '\0';
      args[ct] = NULL; /* no more arguments to this command */
      break;

    default: /* some other character */
      if (start == -1)
        start = i;
      if (inputBuffer[i] == '&')
      {
        *background = 1;
        inputBuffer[i - 1] = '\0';
      }
    } /* end of switch */
  }   /* end of for */

  /**
	 * If we get &, don't enter it in the args array
	 */

  if (*background)
    args[--ct] = NULL;

  args[ct] = NULL; /* just in case the input line was > 80 */

  *argct = ct + 1;

  return 1;

} /* end of parseCommand routine */
