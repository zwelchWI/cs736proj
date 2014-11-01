#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#define SCHED SCHED_FIFO
int orig_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
                       
int my_pthread_create(pthread_t *thread, pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg) {
	struct sched_param param;
	static int ndx = 0;
	static FILE *pFile = NULL;
	static int numThreads = 0;
	static int* prios= NULL;
      	if(prios == NULL){
		pFile = fopen("createPrios.txt","r");
		fscanf(pFile, "%d\n",&numThreads);
		prios = (int *)malloc(numThreads*sizeof(int));
		int ndx2 = 0;
		for(ndx2 = 0;ndx2 < numThreads;ndx2++){
			fscanf(pFile,"%d\n",&(prios[ndx2]));
		}
		fclose(pFile);
	}
	//need this or the real-time scheduling gets ignored
	pthread_attr_setinheritsched(attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(attr, SCHED);
	
	//pick a random priority for the scheduling algorithm chosen
        param.sched_priority = sched_get_priority_min(SCHED)+
                            prios[ndx]; // rand()%sched_get_priority_max(SCHED);

	ndx++;
	pthread_attr_setschedparam(attr, &param);
	return orig_pthread_create(thread,attr,start_routine,arg);
}
