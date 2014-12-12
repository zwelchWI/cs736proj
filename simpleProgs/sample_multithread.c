#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <limits.h>
#include <linux/sched.h>
#define SLEEP 0 
#define NUMTHREADS 10
unsigned int number[NUMTHREADS]; 
pthread_mutex_t lock;

//taken from pthreads man page, prints out the current priority and
//the policy

void print_sched_attr(int num){
	int policy, rtn; 
	struct sched_param param; 
	rtn = pthread_getschedparam(pthread_self(), &policy, &param); 
	if(rtn != 0) perror("Unable to retrieve pthread sched param: "); 

	printf("Scheduler attributes for [%d]:pid %d, thread %ld, policy: %s, priority: %d\n",
		num,
		getpid(),  
		syscall(SYS_gettid), 
		(policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                (policy == SCHED_RR)    ? "SCHED_RR" :
                (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                "???",
                param.sched_priority);
}

void *func1(void *arg)
{
	int val = *((int *) arg); 
	printf("Thread start: number %d\n", val); 
	print_sched_attr(val); 
	
	unsigned int i;
	for(i = 0; i < 10000; i++){
		if(i == 10000 / 2){
			printf("Thread %d is %f way done.\n", val,number[val]/((float)(10000))); 
			print_sched_attr(val);
		}
		pthread_mutex_lock(&lock);
		number[val]++;
		pthread_mutex_unlock(&lock);

	}

	printf("Thread end: number %d\n", val); 

	return NULL;
}

int main(int argc, char* argv[]) {

	pthread_t threads[NUMTHREADS]; 
	pthread_attr_t attr[NUMTHREADS];
	int i;
	for(i = 0; i < NUMTHREADS; i++){
		int ret = pthread_attr_init(&attr[i]); 
		if(ret != 0){
			perror("pthread_attr_init: "); 
			return 1;
		}
	}

	/* Start up threads */
	for(i = 0; i < NUMTHREADS; i++){
		int *arg = malloc(sizeof(*arg)); 
		*arg = i; 
		int ret = pthread_create(&threads[i], &attr[i], func1, arg);
		if(ret != 0){
			perror("pthread_create: "); 
			return 1;
		}
	}
	printf("Done creating threads.\n");
	for(i = 0; i < NUMTHREADS; i++){
		int rtn = pthread_join(threads[i], NULL);
		if(rtn != 0){
			perror("pthread_join: ");
		}
	}
	return 0;
}
