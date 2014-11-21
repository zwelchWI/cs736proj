#include <sys/syscall.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <linux/sched.h>
#include <time.h>
#define SCHED SCHED_RR

int orig_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
                       
int my_pthread_create(pthread_t *thread, pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg) {
	printf("HI\n"); 
	struct sched_param param;
	if(attr == NULL){
		printf("setup attr\n"); 
		pthread_attr_t newattr; 
		 pthread_attr_init(&newattr);
		attr = &newattr; 
	}
	if(attr == NULL) printf("UGH\n");  
	if(thread == NULL) printf("ACK\n"); 
	if(start_routine == NULL) printf("ACK1\n"); 
	if(arg == NULL) printf("ACK2\n"); 
	//need this or the real-time scheduling gets ignored
	pthread_attr_setinheritsched(attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(attr, SCHED);
	
	//pick a random priority for the scheduling algorithm chosen
        param.sched_priority = sched_get_priority_min(SCHED);
                           // rand()%sched_get_priority_max(SCHED);

	pthread_attr_setschedparam(attr, &param);
	int val=orig_pthread_create(thread,attr,start_routine,arg);
	return val;
}

