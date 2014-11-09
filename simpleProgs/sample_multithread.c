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
	for(i = 0; i < INT_MAX; i++){
		if(i == INT_MAX / 2){
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

/*	if(pthread_mutex_init(&lock, NULL) != 0){
		perror("mutex_init: ");
		return 1;
	}
*/



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
	//this must be called otherwise policy and priority are wrong
	/*ret = pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
                if(ret < 0){
                        printf("RETURN VALUE FAIL\n");
                        return 0;
                }
	*/
	/*if(argc>2){
		flag = 1;
		printf("Using alternate scheduler, pid of parent: %d\n", getpid());	
		ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
		if(ret < 0){
			printf("RETURN VALUE FAIL\n");
			return 0;	
		}
		fifo_param.sched_priority =sched_get_priority_min(SCHED_RR);	
		ret = pthread_attr_setschedparam(&attr, &fifo_param);
		if(ret<0){
			printf("FAILEDED\n");
			return 0;
		}
	}*/

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
	/*if(argc>2){
                ret = pthread_attr_setschedpolicy(&attr2, SCHED_RR);
                if(ret < 0){
                        printf("RETURN VALUE FAIL\n");
                        return 0;
                }
		ret = fifo_param2.sched_priority =sched_get_priority_max(SCHED_RR);	
                if(ret<0){
                        printf("FAILEDED\n");
                        return 0;
                }

  		ret = pthread_attr_setschedparam(&attr2, &fifo_param2);

       		if(ret<0){
                	printf("FAILEDED\n");
                	return 0;
        	}

	}*/
	/* Synchronize the completion of each thread. */
	printf("Done creating threads.\n");
	for(i = 0; i < NUMTHREADS; i++){
		int rtn = pthread_join(threads[i], NULL);
		if(rtn != 0){
			perror("pthread_join: ");
		}
	}
//	pthread_mutex_destroy(&lock); 
	return 0;
}
