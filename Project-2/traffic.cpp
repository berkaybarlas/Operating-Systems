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

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

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
int predefinedProbabilties[4] = {0, 0, 0, 0};
double directionProbabilties[4]; //set different probability to each direction 

void *initLane(void *); //Put 1 car in each lane
void *police(void*);
void laneLoop(int ); //Loop for lane threads to spawn cars
void northLaneLoop(); //Lane loop for the special north lane
void addCarToLane(int );
char* convertTime(time_t);
char* normalizeTime(int); 


int carID = 1; // Id of first car
int carNumber = 0; // initial car number
double p;
vector<queue<car> > lanes(4, std::queue<car>());
pthread_mutex_t print_lock;
pthread_mutex_t lane_lock;
pthread_mutex_t car_number;
pthread_cond_t honk;

void printIntersection() {
   cout << "   " << lanes[NORTH].size() << endl;
   cout << lanes[WEST].size() <<"     " << lanes[EAST].size() << endl;
   cout << "   " << lanes[SOUTH].size() << endl;
}

void cmdline(int argc, char *argv[], double &p, int &s, int &t) {
int flags, opt;
   s = 100;
   p = 1.0;
   t = 0;
    flags = 0;
    while ((opt = getopt(argc, argv, "s:p:t:n:e:d:w:")) != -1) {
        switch (opt) {
        case 's':
            s = atoi(optarg);
            cout << " S:  "<< optarg << endl;
            break;
        case 'p':
            p = atof(optarg);
            cout << " P:  " << optarg << endl;
            break;
         case 't':
            t = atof(optarg);
            cout << " T:  " << optarg << endl;
            break;
         case 'n':
            directionProbabilties[0] = atof(optarg);
            predefinedProbabilties[0] = 1;
            cout << " North:  " << optarg << endl;
            break;
         case 'e':
            directionProbabilties[1] = atof(optarg);
            predefinedProbabilties[1] = 1;
            cout << " East:  " << optarg << endl;
            break;
         case 'd':
            directionProbabilties[2] = atof(optarg);
            predefinedProbabilties[2] = 1;
            cout << " South:  " << optarg << endl;
            break;
         case 'w':
            directionProbabilties[3] = atof(optarg);
            predefinedProbabilties[3] = 1;
            cout << " West:  " << optarg << endl;
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
   
   if (pthread_mutex_init(&print_lock, NULL) != 0 && pthread_mutex_init(&lane_lock, NULL) && pthread_mutex_init(&car_number, NULL))   { 
        printf("\n mutex init has failed\n"); 
        return 1; 
   } 
   pthread_cond_init (&honk, NULL);

   cmdline(argc, argv, p, s, t);
   time_t startTime = time(0);
   clock_t start = clock();
   clock_t prev_sec = clock();
   int second = 0;
   double duration = 0;
   
   cout << "Args:" << p <<" "<< s << endl;

   for( int i = 0; i < LANE_NUMBER; i++ ) {
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
      if( clock() - prev_sec > ONE_SECOND) {
         prev_sec = ++second * ONE_SECOND; 
         cout << second << " second elapsed" << clock() << " " << convertTime(time(NULL)) << endl;
      }
      if(t == second) {
         t++; 
         printIntersection();

      }
      duration = (clock() - start ) / (double) CLOCKS_PER_SEC;
   }
   printIntersection();
   
   cout << "finished computation at " << clock() << " elapsed time: " << duration << "s\n";
   pthread_mutex_destroy(&print_lock); 
   pthread_mutex_destroy(&lane_lock); 
   pthread_mutex_destroy(&car_number); 
   pthread_cond_destroy(&honk);

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

   fprintf(policeLog, "Time \t Event \n");
   fprintf(policeLog, "_______________\n");
   
   int turnIndex = 0;
   
	while(true) {
      // Priority order 
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

      // Start playing with cell phone
      if(maxNumberOfCars == 0 ) {
         fprintf(policeLog, "%s \t %s \n", convertTime(time(NULL)), "Cell Phone");
         // Use semaphore and wait for cars
         while (carNumber <= 0) {
            pthread_cond_wait(&honk,&lane_lock);
         }
         fprintf(policeLog, "%s \t %s \n", convertTime(time(NULL)), "Honk");
         pthread_sleep(3); 
      }

      int maxWait = 0;
  	   for(int i = 0; i < LANE_NUMBER; i++) {
  	  	   if(!lanes[i].empty()) {
		  	   car c = lanes[i].front();
		  	   time_t currentTime = time(NULL);
		  	   int waitTime = ( currentTime - c.arrivalTime );
		  	   if(waitTime > maxWait){
		  	   	maxWait = waitTime;
		  	   	if(waitTime > 20){
		  	   		turnIndex = i;
		  	   	}
		  	   }
      	}
      }
      if(maxNumberOfCars != 0) {
         //cout << "The biggest size: " << maxNumberOfCars << " " <<  turnIndex <<endl;
         car crossingCar = (lanes[turnIndex].front());
         lanes[turnIndex].pop();
         pthread_mutex_lock(&car_number);
         carNumber--;
         pthread_mutex_unlock(&car_number);
         maxNumberOfCars = 0;
         time_t currentTime = time(NULL);
         currentTimeInfo = localtime(&currentTime);
         arrivalTimeInfo = localtime(&crossingCar.arrivalTime);

         int waitTime = ( currentTime - crossingCar.arrivalTime );
         fprintf(carLog, "%d \t\t %c \t\t\t %s \t\t %s \t\t %d\n", 
         crossingCar.carID, 
         crossingCar.direction,  
         convertTime(crossingCar.arrivalTime),
         convertTime(currentTime),
         waitTime);

         cout << "Crossing Car: " << crossingCar.carID << "\t" 
         << crossingCar.direction << "\t" 
         << convertTime(crossingCar.arrivalTime) << "\t" 
         << convertTime(currentTime) << "\t"
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

char* convertTime(time_t fullTime) {
   struct tm *localTm = localtime(&fullTime);
   char* Time = (char*) malloc(sizeof(char) * 8);
   int hour = localTm->tm_hour;
   int min = localTm->tm_min;
   int sec = localTm->tm_sec;
   sprintf(Time, "%s:%s:%s", normalizeTime(hour), normalizeTime(min), normalizeTime(sec));
   return Time;
}

char* normalizeTime(int timeInt) {
   char* timeString = (char*) malloc(sizeof(char) * 2);
   if(timeInt < 10) 
      sprintf(timeString, "0%d", timeInt);
   else
      sprintf(timeString, "%d", timeInt);
   return timeString;
}

void *initLane(void *laneIndptr) {
	int laneInd = *((int*)laneIndptr);
	pthread_mutex_lock(&lane_lock);
	addCarToLane(laneInd);
	pthread_mutex_unlock(&lane_lock);
	if(laneInd == 0){
		northLaneLoop();
	}
	else{
		laneLoop(laneInd);
	}
   return NULL;
}

void laneLoop(int laneInd) {
	pthread_sleep(1);
	pthread_mutex_lock(&lane_lock);
	double randNum = (double)rand() / (double)RAND_MAX;
	//cout << randNum << endl;
   int newCar = 0 ;
   if(predefinedProbabilties[laneInd] == 1) {
      if(randNum < directionProbabilties[laneInd]) {
         newCar = 1;
      }
   } else if(randNum < p){
	   newCar = 1;
	}
   // Add new car to lane
   if(newCar) {
      addCarToLane(laneInd);
   }
	pthread_mutex_unlock(&lane_lock);
	laneLoop(laneInd);
}

void northLaneLoop() {
	pthread_sleep(1);
	pthread_mutex_lock(&lane_lock);
	double randNum = (double)rand() / (double)RAND_MAX;
	//cout << randNum << endl;
   int newCar = 0 ;
   if(predefinedProbabilties[0] == 1) {
      if(randNum < directionProbabilties[0]) {
         newCar = 1;
      }
   } else if(randNum > p){
		newCar = 1;
	} 
   if(newCar) {
      addCarToLane(0);
      pthread_cond_signal(&honk);
		pthread_mutex_unlock(&lane_lock);
   } else {
		pthread_mutex_unlock(&lane_lock);
		pthread_sleep(19);
	}
	northLaneLoop();
}

void addCarToLane(int laneInd) {
   car c = {carID++, directions[laneInd], time(NULL), 0, 0};
	lanes[laneInd].push(c);
   pthread_mutex_lock(&car_number);
   if(carNumber == 0 ) {
      pthread_cond_signal(&honk);
   }
   carNumber++;
   pthread_mutex_unlock(&car_number);
}

