# Project 2 

### We implemented all parts of the project.

We have included a Makefile with our project.
It can be used to compile the code with the command: ``` make traffic ```
To run the program with predefined arguments, you can use "make run"
To run the program with custom arguments, specify them in the following format:

``` 
./traffic.o -p 0.5 -s 5 -t 0
```

Our program starts with main, creates 4 threads for the traffic lanes, and 1 for the police which controls the intersection passing. 
The main thread stays alive and prints the intersection every second from given time with -t argument.
When the specified simulation time ends, main returns with exit(0), and all threads die as the process terminates. Therefore, we didn't use pthread_exit() . 

We used 3 mutex locks to achieve thread synchronization and protect shared data.
* A lane_lock to restrict access to the traffic lane data structures to one thread at a time.
* A car_number to restrict access to a variable holding the number of cars.

We also used a honk conditional variable to do the signalling in the third part.


### We created two log samples with using 
``` 
make traffic && ./traffic.o -p 0.4 -s 60 -t 0 
```
command and stored them in SampleLogFiles folder.
