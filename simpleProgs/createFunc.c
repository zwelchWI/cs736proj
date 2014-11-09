#include <sys/syscall.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define SCHED SCHED_RR
int orig_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
pthread_mutex_t lock;
FILE *logFile = NULL;                      
int my_pthread_create(pthread_t *thread, pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg) {
	struct sched_param param;
	static int ndx = 0;
	static FILE *pFile = NULL;
	static int numThreads = 0;
	static int* prios= NULL;

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
        if(logFile){
		fprintf(logFile, "about to call original pthread create with %d prio on fn %p\n",
				param.sched_priority,start_routine);
		fflush(logFile);
	} 
	int val=orig_pthread_create(thread,attr,start_routine,arg);
	return val;
}


void randPrio(){
	pthread_t this = pthread_self();
  	struct sched_param fifo_param;
	
         fifo_param.sched_priority =sched_get_priority_min(SCHED)+
                                    rand()%(sched_get_priority_max(SCHED)-sched_get_priority_min(SCHED));
         if(pthread_setschedparam(this,SCHED,&fifo_param) != 0)
			perror("couldn't set sched:");
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

char *get_formatted_time() {
   time_t curtime = time(NULL);
   char *fmt_time = asctime(localtime(&curtime));
   char *nl;
   if(nl = strchr(fmt_time, '\n'))
      *nl = 0;
   return fmt_time;
}

void trace_entry_func(char *func_name, char *desc_line, int num_func_args, 
                      void *func_arg1, int func_arg1_type) {
   char line[200];
   char tempstr[100];
    
   if(logFile == NULL){
      logFile = fopen("/tmp/threadLog.txt","a");
   }
   sprintf(line, "[TRACE_ENTRY- thread:%d, func: %s, num_args: %d",
           syscall(SYS_gettid),func_name, num_func_args);

   if(func_arg1_type == 1) {
      sprintf(tempstr, ", arg1: %d", (int)func_arg1);
      strcat(line, tempstr);
   }
   sprintf(tempstr, ", %s, %s]\n", desc_line, get_formatted_time());
   strcat(line, tempstr);
   pthread_mutex_lock(&lock);
   fprintf(logFile, line);
   fflush(logFile);
   pthread_mutex_unlock(&lock);
}

void trace_exit_func(char *func_name, char *desc_line, void *ret_val,
                     int ret_val_type) {
   char line[200];
   char tempstr[100];

   if(logFile == NULL){
      logFile = fopen("/tmp/threadLog.txt","a");
   }

   sprintf(line, "[TRACE_EXIT- thread:%d, func: %s",syscall(SYS_gettid), func_name);
   if(ret_val_type == 1) {
      sprintf(tempstr, ", ret_val: %d", (int)ret_val);
      strcat(line, tempstr);
   }
   sprintf(tempstr, ", %s, %s]\n", desc_line, get_formatted_time());
   strcat(line, tempstr);
   pthread_mutex_lock(&lock);
   fprintf(logFile, line);
   fflush(logFile);
   pthread_mutex_unlock(&lock);
}

