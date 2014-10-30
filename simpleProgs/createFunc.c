#include <pthread.h>
#include <stdlib.h>

#define SCHED SCHED_FIFO

int orig_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
                       
int my_pthread_create(pthread_t *thread, pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg) {
	struct sched_param param;

	//need this or the real-time scheduling gets ignored
	pthread_attr_setinheritsched(attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(attr, SCHED);
	
	//pick a random priority for the scheduling algorithm chosen
        param.sched_priority = sched_get_priority_min(SCHED)+
                                 rand()%sched_get_priority_max(SCHED);
	pthread_attr_setschedparam(attr, &param);
	return orig_pthread_create(thread,attr,start_routine,arg);
}
