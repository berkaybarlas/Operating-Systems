#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <queue>

using namespace std;

#define NUM_THREADS 5

void initLanes(vector<queue<int>> lanes);

struct thread_data {
   int  thread_id;
   char *message;
};

void *PrintHello(void *threadarg) {
   struct thread_data *my_data;
   my_data = (struct thread_data *) threadarg;

   cout << "Thread ID : " << my_data->thread_id ;
   cout << " Message : " << my_data->message << endl;

   pthread_exit(NULL);
}

int main () {
   pthread_t threads[NUM_THREADS];
   struct thread_data td[NUM_THREADS];
   int rc;
   int i;
   vector<queue<int>> lanes;
   
   initLanes(lanes);

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

void initLanes(vector<queue<int>> lanes){
	for(int dir = 0; dir < 0; dir++){
		queue<int> q = lanes[dir];
		q.push((dir+2)%4);
	}
}
