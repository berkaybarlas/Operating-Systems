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
#define LANE_NUMBER 4
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

char directions[4] = {'N', 'E', 'S', 'W'};

void *initLane(void *laneIndptr); //Put 1 car in each lane
void laneLoop(int laneInd); //Loop for lane threads to spawn cars

int carID = 0;
double p;
vector<queue<car> > lanes(4, std::queue<car>());
pthread_mutex_t print_lock;
pthread_mutex_t lane_lock;

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
   int s;
   
   if (pthread_mutex_init(&print_lock, NULL) != 0 && pthread_mutex_init(&lane_lock, NULL)) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
   } 
   
   cmdline(argc, argv, p, s);
   time_t startTime = time(0);
   clock_t start = clock();
   clock_t prev_sec = clock();
   int second = 0;
   double duration = 0;
   
   cout << "Args:" << p <<" "<< s << endl;

   for( int i = 0; i < LANE_NUMBER; i++ ) {
      cout <<"main() : creating thread, " << i << endl;
      int *index = (int*) malloc(sizeof(int));
      *index = i;
      rc = pthread_create(&threads[i], NULL, initLane, (void *)(index));
      
      if (rc) {
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
      }
   }
   // Create Police Thread 
   
   // Police prototype 
   // N > E > S > W
   int maxNumberOfCars = 0;
   int turnIndex = 0;
   

   while(duration < s) {
      // Make things
      if(clock() - prev_sec > ONE_SECOND) {
         prev_sec = ++second * ONE_SECOND; 
         cout << second << " second elapsed" << clock() << endl; 
         printIntersection(lanes[0].size(),lanes[1].size(),lanes[2].size(),lanes[3].size());
      
      pthread_mutex_lock(&lane_lock);
      for(int i = 0; i < LANE_NUMBER; i++) {
         //
         int numberOfCars = lanes[i].size();
         if(numberOfCars > maxNumberOfCars) {
            turnIndex = i; 
            maxNumberOfCars = numberOfCars;
         }
      }
      cout << "The biggest size: " << maxNumberOfCars << " " <<  turnIndex <<endl;
      if(maxNumberOfCars != 0) {
         car crossingCar = (lanes[turnIndex].front());
         lanes[turnIndex].pop();
         cout << "Crossing Car: " << crossingCar.carID << endl;
      }
      pthread_mutex_unlock(&lane_lock);
      }
      duration = (clock() - start ) / (double) CLOCKS_PER_SEC;
   }
   
   cout << "finished computation at " << clock() << " elapsed time: " << duration << "s\n";
   pthread_mutex_destroy(&print_lock); 
   pthread_mutex_destroy(&lane_lock); 
}

void *initLane(void *laneIndptr){
	int ind = *((int*)laneIndptr);
	pthread_mutex_lock(&lane_lock);
	car c = {carID++, directions[ind], clock(), 0, 0};
	lanes[ind].push(c);
	pthread_mutex_unlock(&lane_lock);
	laneLoop(ind);
}

void laneLoop(int laneInd){
	pthread_sleep(1);
	double randNum = (double)rand() / (double)RAND_MAX;
	if(randNum < p){
		pthread_mutex_lock(&lane_lock);
		car c = {carID++, directions[laneInd], clock(), 0, 0};
		lanes[laneInd].push(c);
		pthread_mutex_unlock(&lane_lock);
	}
	laneLoop(laneInd);
}
