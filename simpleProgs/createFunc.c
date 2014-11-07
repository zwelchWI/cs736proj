#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#define SCHED SCHED_RR
int orig_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
                       
int my_pthread_create(pthread_t *thread, pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg) {
	struct sched_param param;
	static int ndx = 0;
	static FILE *pFile = NULL;
	static int numThreads = 0;
	static int* prios= NULL;
	static FILE *logFile = NULL;

	if(logFile == NULL){
		logFile = fopen("/tmp/threadLog.txt", "a");
	}
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
	fprintf(logFile, "about to call original pthread create\n"); 
	fflush(logFile); 
	int val=orig_pthread_create(thread,attr,start_routine,arg);
	return val;
}


void randPrio(){
	pthread_t this = pthread_self();
  	struct sched_param fifo_param;
	
         fifo_param.sched_priority =sched_get_priority_min(SCHED)+
                                    rand()%sched_get_priority_max(SCHED);
         if(pthread_setschedparam(this,SCHED,&fifo_param) != 0)
			perror("couldn't set sched:");
	print_sched_attr(); 
	usleep(1);
}



pid_t orig_fork(); 

pid_t my_fork(){
	struct sched_param param;
	static int ndx = 0;
	static FILE *pFile = NULL;
	static int numThreads = 0;
	static int* prios= NULL;
	static FILE *logFile = NULL;

	if(logFile == NULL){
		logFile = fopen("/tmp/threadLog.txt", "a");
	}
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
	//pick a random priority for the scheduling algorithm chosen
        param.sched_priority = sched_get_priority_min(SCHED)+
                            prios[ndx]; // rand()%sched_get_priority_max(SCHED);

	ndx++;
	int rtn = sched_setscheduler(getpid(), SCHED, &param); 
	if(rtn != 0) printf("problems\n"); 
	return orig_fork();
}
