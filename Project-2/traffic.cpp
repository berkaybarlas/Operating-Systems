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

void initLanes(vector<queue<int> > lanes);
struct car {
   int carID;
   char direction;
   time_t arrivalTime;
   time_t crossTime;
   time_t waitTime;
};

struct thread_data {
   int  thread_id;
   char *message;
};

void initLanes(vector<queue<car>> *lanes); //Put 1 car in each lane
void laneLoop(queue<car> *lane, double p, char dir); //Loop for lane threads to spawn cars

int carID = 0;

void *PrintHello(void *threadarg) {
   struct thread_data *my_data;
   my_data = (struct thread_data *) threadarg;

   cout << "Thread ID : " << my_data->thread_id ;
   cout << " Message : " << my_data->message << endl;

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
   double p;
   int s;
   vector<queue<int> > lanes;
   
   initLanes(&lanes);
   
   cmdline(argc, argv, p, s);
   
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
   pthread_exit(NULL);
}

void initLanes(vector<queue<int> > *lanes){
	for(int dir = 0; dir < 0; dir++){
		queue<car> q = (*lanes)[dir];
		car c {carID++, 'N', time(NULL), 0, 0};
		q.push(c);
	}
}
