README

# COMP 304 Spring 2019 Project-1
### Implemented by:
- Berkay Barlas - 54512
- Ege Onat Özsüer - 60210

### Included files:
* shelldon.c : the shell code
* oldestChilde.c : the kernel module
* Makefile: make file for shelldon.c and kernel module
* some screenshots

# Project 1 

## Part-1
- shell commands

In this part of the code we added execv funcionality to the shelldon program. If the user enters the command with “./” at the start, we use the args as given and put them inside a call to the execv function. If there is no “./” at the start, we instead put a “/bin/” string at the beginning of their first arg, and assume the command they are requesting is in that folder.

We also added functionality for the “cd” command. This is done with a chdir() function call instead of using execv.

## Part-2
- I/O redirection

In this part we limplemented the redirection operators, “>” and “>>”. These operators redirect the output of the programs to be executed, writing their outputs to the given files. They support truncation and appending.

We also looked at the history functionality in the book and implemented our own. This works by keeping the 10 last arguments given by the user inside the memory. Whenever the “history” command is entered, these 10 arguments are printed, with numbers next to them.

The user can also use the “ !X “ commands to execute these previously entered commands inside the history, where X must be the number corresponding to the line number associated with the printed args.

## Part-3
- codeSearch "\<word\>"
- codeSearch -r "\<word\>" 
- codeSearch "\<word\>" -f \<file\>

codeSearch command opens every file in current directory and reads them line by line if it finds a match it prints that line. 

eg.
```
codesearch "while"

92: ./shelldon.c -> while(a,b);
```

We also chose "write" as our defined command. Write uses an external program called figlet to echo the second argument given, but in a prettified form.

## Part-4
- oldestChild

In this part of our project, we developed a kernel module that prints the PIDs and executable names of the oldest children of current
process and repeat that process for every child of root process.
The kernel module can be compiled with makefile using
- make all

It can be loaded by using in our shell
oldestchild <PID Number>

