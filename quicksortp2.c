#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

#define THREADS 10
#define ELEMCOUNT 100000


//int a[] = {3,7,12,4,2,5};

//int a[] = {1,3,2,4,8,433,22,57,6,4,2,-1,-5,-9,32,5,3,6,7,6};

int a[ELEMCOUNT];
int threadCount = 0;

//controls number of threads spawned
//sem_t sema;
pthread_mutex_t lockA;

typedef struct boundaries {
	int* pivot;
	int* end;
} boundaries;

void swap(int* a, int* b) {
	int holder = *a;
	*a = *b;
	*b = holder;
}

//for sequential computation
void qss(int* pivot, int* end) {
	int* wall = pivot + 1; // start at next element over
	int* curr = pivot + 1;

	//end condition
	if(pivot + 1 == end || pivot == end) {
		return;
	}

	for(;curr != end; curr++) {
		if(*curr < *pivot) {
			swap(curr, wall);
			wall += 1;
		}

	}

	swap(wall - 1, pivot);

	qss(pivot, wall);
	qss(wall, end);
}

void* quicksort(void *tb) {

	boundaries threadBound = *((boundaries*)tb);

	// int* begin = threadBound->pivot;
	// int* end = threadBound->end;
	//end condition
	if(threadBound.pivot + 1 == threadBound.end ||
	 threadBound.pivot == threadBound.end) {
	 	pthread_exit(NULL);
	}

	//make sure to swap wall - 1 with pivot
	int* wall = threadBound.pivot + 1; // start at next element over
	int* curr = threadBound.pivot + 1;

	for(;curr != threadBound.end; curr++) {
		if(*curr < *threadBound.pivot) {
			swap(curr, wall);
			wall += 1;
		}

	}

	swap(wall - 1, threadBound.pivot);

	//prints the state of array on this itteration
	// int* i = threadBound.pivot;
	// for(;i != threadBound.end; ++i) {
	// 	printf("%d, ", *i);
	// }

	// printf("\n");

	//these could be getting destroyed on each passthrough of recursion
	//cause they are being redefinded
	boundaries leftBound;
	leftBound.pivot = threadBound.pivot;
	leftBound.end = wall; //wall is where the next thread begins

	boundaries rightBound;
	rightBound.pivot = wall;
	rightBound.end = threadBound.end;

	pthread_t leftThread;
	pthread_t rightThread;
	//tests if the thread was created
	// int leftFlag = 0;
	// int rightFlag = 0;


	//want to use parallelism by creating both threads
	//before causing this thread to wait
	if (threadCount < THREADS){
		//pthread_mutex_lock(&lockA);
		threadCount++;
		//pthread_mutex_lock(&lockA);
		pthread_create(&leftThread, NULL, quicksort, (void*)&leftBound);
	}

	if (threadCount < THREADS) {
		//pthread_mutex_lock(&lockA);
		threadCount++;
		//pthread_mutex_lock(&lockA);
		pthread_create(&rightThread, NULL, quicksort, (void*)&rightBound);
	} 
	if (threadCount >= THREADS) { //run ssequentially
		qss(threadBound.pivot, wall);
		qss(wall, threadBound.end);
	}

	//join threads if they were created
	pthread_join(leftThread, NULL);
	pthread_join(rightThread, NULL);

	pthread_exit(NULL);

}

int main() {

	srand(time(NULL));
	int generate = 0;
	for(;generate < ELEMCOUNT; ++generate) {
		int elem = rand();
		a[generate] = elem;
	}

	boundaries threadZero;
	threadZero.pivot = a;
	threadZero.end = a + sizeof(a)/4;
	clock_t start, end;
	double cpu_time_used;

	pthread_t topThread;

	pthread_mutex_init(&lockA, NULL);
	//DO NOT TYPECAST THE FUNCTION POINTER
	start = clock();
	threadCount++;
	pthread_create(&topThread, NULL, quicksort, (void*)&threadZero);

	//join forces the parent thread to wait for the child thread to terminate before continuing
	pthread_join(topThread, NULL);
	pthread_mutex_destroy(&lockA);

	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	//need to make sure that all sections have found the location of their pivot points
	//before continuing with the partition, this means barriers



	// int *i = a;
	// for(;i != a + sizeof(a)/4; ++i) {
	// 	printf("%d, ", *i);
	// }

	printf("\n");

	printf("parallel time taken: %f\n", cpu_time_used);

}