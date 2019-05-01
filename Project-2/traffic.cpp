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
#include <cstring>

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
void *police(void*);
void laneLoop(int laneInd); //Loop for lane threads to spawn cars
void northLaneLoop(); //Lane loop for the special north lane
char* convertTime(struct tm *fullTime);

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

void printIntersection() {
   cout << "   " << lanes[0].size() << endl;
   cout << lanes[3].size() <<"     " << lanes[1].size() << endl;
   cout << "   " << lanes[2].size() << endl;
}

void cmdline(int argc, char *argv[], double &p, int &s, int &t ) {
int flags, opt;
   s = 100;
   p = 1.0;
   t = 0;
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
   int t;
   srand(time(0));
   
   if (pthread_mutex_init(&print_lock, NULL) != 0 && pthread_mutex_init(&lane_lock, NULL)) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
   } 
   
   cmdline(argc, argv, p, s, t);
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
   rc = pthread_create(&threads[LANE_NUMBER], NULL, police, NULL);

   while(duration < s) {
      // Make things
      if(clock() > ONE_SECOND * t && clock() - prev_sec > ONE_SECOND) {
         prev_sec = ++second * ONE_SECOND; 
         cout << second << " second elapsed" << clock() << endl; 
         printIntersection();

      }
      duration = (clock() - start ) / (double) CLOCKS_PER_SEC;
   }
   printIntersection();
   
   cout << "finished computation at " << clock() << " elapsed time: " << duration << "s\n";
   pthread_mutex_destroy(&print_lock); 
   pthread_mutex_destroy(&lane_lock); 
   // Add exit to close all threads
}

void *police(void *) {
   // car.log -> intersection time 
   // police.log -> keep cell phone usage and honk 
   FILE *carLog;
   FILE *policeLog;
   carLog = fopen("./car.log", "w+");
   policeLog = fopen("./police.log", "w+");
   fprintf(carLog, "CarID \t Direction \t Arrival-Time \t Cross-Time \t Wait-Time\n");
   fprintf(carLog, "_________________________________________________________________\n");
   
   int turnIndex = 0;
   
   while(true) {
      // Police prototype 
      // N > E > S > W
      int maxNumberOfCars = 0;
      int turnIndex = 0;
      struct tm *currentTimeInfo;
      struct tm *arrivalTimeInfo;

      pthread_mutex_lock(&lane_lock);
      for(int i = 0; i < LANE_NUMBER; i++) {
         //
         int numberOfCars = lanes[i].size();
         if(numberOfCars > maxNumberOfCars) {
         	if(numberOfCars > 4 || lanes[turnIndex].empty()){
            turnIndex = i;          	
         	}
            maxNumberOfCars = numberOfCars;
         }
      }
      if(maxNumberOfCars != 0) {
         //cout << "The biggest size: " << maxNumberOfCars << " " <<  turnIndex <<endl;
         car crossingCar = (lanes[turnIndex].front());
         //printIntersection();
         lanes[turnIndex].pop();
         //printIntersection();
         maxNumberOfCars = 0;
         time_t currentTime = time(NULL);
         currentTimeInfo = localtime(&currentTime);
         arrivalTimeInfo = localtime(&crossingCar.arrivalTime);

         int waitTime = ( currentTime - crossingCar.arrivalTime );
         fprintf(carLog, "%d \t\t %c \t\t\t %s \t\t %s \t\t %d\n", 
         crossingCar.carID, 
         crossingCar.direction, 
         convertTime(arrivalTimeInfo), 
         convertTime(currentTimeInfo), waitTime);
         cout << "Crossing Car: " << crossingCar.carID << "\t" 
         << crossingCar.direction << "\t" 
         << convertTime(arrivalTimeInfo) << "\t" 
         << convertTime(currentTimeInfo) << "\t"
         << waitTime << "\t"  << endl;
         pthread_mutex_unlock(&lane_lock);
         pthread_sleep(1);
      } else {
         pthread_mutex_unlock(&lane_lock);
      }
   }
   fclose(carLog);
   fclose(policeLog);
}

char* convertTime(struct tm *fullTime) {
   char* Time = (char*) malloc(sizeof(char) * 8);
   int hour = fullTime->tm_hour;
   int min = fullTime->tm_min;
   int sec = fullTime->tm_sec;
   if(sec < 10) 
      sprintf(Time, "%d:%d:0%d", hour, min, sec);
   else
      sprintf(Time, "%d:%d:%d", hour, min, sec);
   return Time;
}

void *initLane(void *laneIndptr) {
	int ind = *((int*)laneIndptr);
	pthread_mutex_lock(&lane_lock);
	car c = {carID++, directions[ind], time(NULL), 0, 0};
	lanes[ind].push(c);
	pthread_mutex_unlock(&lane_lock);
	if(ind == 0){
		northLaneLoop();
	}
	else{
		laneLoop(ind);
	}
}

void laneLoop(int laneInd) {
	pthread_sleep(1);
	pthread_mutex_lock(&lane_lock);
	double randNum = (double)rand() / (double)RAND_MAX;
	//cout << randNum << endl;
	if(randNum < p){
		car c = {carID++, directions[laneInd], time(NULL), 0, 0};
		lanes[laneInd].push(c);
	}
	pthread_mutex_unlock(&lane_lock);
	laneLoop(laneInd);
}

void northLaneLoop() {
	pthread_sleep(1);
	pthread_mutex_lock(&lane_lock);
	double randNum = (double)rand() / (double)RAND_MAX;
	//cout << randNum << endl;
	if(randNum > p){
		car c = {carID++, directions[0], clock(), 0, 0};
		lanes[0].push(c);
		pthread_mutex_unlock(&lane_lock);
	} else {
		pthread_mutex_unlock(&lane_lock);
		pthread_sleep(19);
	}
	northLaneLoop();
}
