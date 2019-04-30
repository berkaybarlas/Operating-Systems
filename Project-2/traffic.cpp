#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <queue>
#include <ctime>
#include <random>
#include "pthread_sleep.c"
#include <getopt.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

#define NUM_THREADS 5
#define LANE_NUMBER 2
#define ONE_SECOND 1000000

struct car {
   int carID;
   char direction;
   time_t arrivalTime;
   time_t crossTime;
   time_t waitTime;
};

struct thread_data {
   int  thread_id;
   const char *message;
};

void initLanes(vector<queue<car> > *lanes); //Put 1 car in each lane
void *laneLoop(int laneInd); //Loop for lane threads to spawn cars

int carID = 0;
double p;
vector<queue<car> > lanes(4, std::queue<car>());
pthread_mutex_t print_lock;

void *PrintHello(void *threadarg) {
   struct thread_data *my_data;
   my_data = (struct thread_data *) threadarg;
   pthread_mutex_lock(&print_lock);
   cout << "Thread ID : " << my_data->thread_id ;
   cout << " Message : " << my_data->message << endl;
   pthread_mutex_unlock(&print_lock); 
   pthread_exit(NULL);
}

void printIntersection(int n, int w, int e, int s) {
   cout << "   " << n << endl;
   cout << w <<"     " << e << endl;
   cout << "   " << s << endl;
}

void cmdline(int argc, char *argv[], double &p, int &s ) {
int flags, opt;
   s = 100;
   p = 1.0;
    flags = 0;
    while ((opt = getopt(argc, argv, "s:p:")) != -1) {
        switch (opt) {
        case 's':
            s = atoi(optarg);
            cout << " S:  "<< optarg << endl;
            break;
        case 'p':
            p = atof(optarg);
            cout << " P:  " << optarg << endl;
            break;
        default: /* 'Error' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

int main (int argc, char *argv[]) {
   pthread_t threads[NUM_THREADS];
   struct thread_data td[NUM_THREADS];
   int rc;
   int i;
   int s;

   if (pthread_mutex_init(&print_lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
   } 

   initLanes(&lanes);
   
   cmdline(argc, argv, p, s);
   time_t startTime = time(0);
   clock_t start = clock();
   clock_t prev_sec = clock();
   int second = 0;
   double duration = 0;
   
   cout << "Args:" << p <<" "<< s << endl;

   for( i = 0; i < NUM_THREADS; i++ ) {
      cout <<"main() : creating thread, " << i << endl;
      td[i].thread_id = i;
      td[i].message = "This is message";
      rc = pthread_create(&threads[i], NULL, PrintHello, (void *)&td[i]);
      
      if (rc) {
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
      }
   }
   // Police prototype 
   // N > E > S > W
   int maxNumberOfCars = 0;
   for(int dir = 0; dir < LANE_NUMBER; dir++) {
		//
      int numberOfCars = 1;
      if(numberOfCars > maxNumberOfCars) {
         maxNumberOfCars = numberOfCars;
      }
	}
   
   for(int dir = 0; dir < LANE_NUMBER; dir++) {
		//
      // check if it equals maxNumberOfCars
      // if it is equal stop for loop 
      // N > E > S > W
	}

   while(duration < s) {
      // Make things
      if(clock() - prev_sec > ONE_SECOND) {
         prev_sec = ++second * ONE_SECOND; 
         cout << second << " second elapsed" << clock() << endl; 
         printIntersection(lanes[0].size(),lanes[1].size(),lanes[2].size(),lanes[3].size());
      }
      duration = (clock() - start ) / (double) CLOCKS_PER_SEC;
   }
   
   cout << "finished computation at " << clock() << " elapsed time: " << duration << "s\n";
   pthread_mutex_destroy(&print_lock); 
   
}

void initLanes(vector<queue<car> > *lanes) {
	for(int dir = 0; dir < LANE_NUMBER; dir++) {
		car c = {carID++, 'N', time(NULL), 0, 0};
		(*lanes)[dir].push(c); 
	}
}

void *laneLoop(int laneInd){
	pthread_sleep(1);
	double randNum = (double)rand() / (double)RAND_MAX;
	if(randNum < p){
		car c = {carID++, 'N', clock(), 0, 0};
		lanes[laneInd].push(c);
		cout << "Pushed car to lane " << laneInd << endl;
	}
	laneLoop(laneInd);
}
