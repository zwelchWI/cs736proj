#include <pthread.h>
int orig_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
                       
int my_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg) {
	printf("PLZ BE HERE\n");
	struct sched_param fifo_param;
//	pthread_attr_setinheritsched(attr,PTHREAD_EXPLICIT_SCHED);
//	pthread_attr_setschedpolicy(attr, SCHED_RR);
//	fifo_param.sched_priority =sched_get_priority_min(SCHED_RR);	
//	pthread_attr_setschedparam(attr, &fifo_param);
	return orig_pthread_create(thread,attr,start_routine,arg);
}
