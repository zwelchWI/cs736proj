#include <sys/syscall.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <linux/sched.h>
#include <time.h>
#include <string.h>

int orig_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
//SCHED_RESET_ON_FORK
//why this can't be SCHED_OTHER nobody knows...
int schedule = SCHED_RR;  
FILE *logFile = NULL;                      

int my_pthread_create(pthread_t *thread, pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg) {
	struct sched_param param;
	static int ndx = 0;
	static FILE *pFile = NULL;
	static int numThreads = 0;
	static int* prios= NULL;

	//setup to get the schedule type and threads for first time
	//this function is called
	if(logFile == NULL){
		logFile = fopen("/tmp/threadLog.txt", "a");
	}

	//checking the arguments passed in
	if(attr == NULL){
		pthread_attr_t newattr;
                pthread_attr_init(&newattr);
                attr = &newattr;
        }
	if(attr == NULL || thread == NULL || start_routine == NULL){
		fprintf(logFile, "null passed to needed argument in my_pthread_create\n"); 
		exit(1); 
	}
	
      	if(prios == NULL){
		pFile = fopen("createPrios.txt","r");
		fscanf(pFile, "%d,%d\n",&schedule, &numThreads);
		printf("schedule %d threads %d\n", schedule, numThreads); 
		prios = (int *)malloc(numThreads*sizeof(int));
		int ndx2 = 0;
		for(ndx2 = 0;ndx2 < numThreads;ndx2++){
			fscanf(pFile,"%d\n",&(prios[ndx2]));
		}
		fclose(pFile);
	}
	//need this or the real-time scheduling gets ignored, except for SCHED_OTHER
	if(schedule != 0) pthread_attr_setinheritsched(attr,PTHREAD_EXPLICIT_SCHED);

	pthread_attr_setschedpolicy(attr, schedule);

	//pick a random priority for the scheduling algorithm chosen
       	param.sched_priority = sched_get_priority_min(schedule)+
       		 prios[ndx]; // rand()%sched_get_priority_max(schedule);

	ndx++;
	pthread_attr_setschedparam(attr, &param);
	return orig_pthread_create(thread,attr,start_routine,arg);
}


void randPrio(){
	//SCHED_OTHER should not choose a random priority
	if(schedule == 0) return;
 
	pthread_t this = pthread_self();
  	struct sched_param fifo_param;
	
	fifo_param.sched_priority =sched_get_priority_min(schedule)+
			    rand()%(sched_get_priority_max(schedule)-sched_get_priority_min(schedule));

        if(pthread_setschedparam(this,schedule,&fifo_param) != 0)
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
	ndx++;
	if(ndx < numThreads){
		//pick a random priority for the scheduling algorithm chosen
		param.sched_priority = sched_get_priority_min(schedule)+
                            prios[ndx]; // rand()%sched_get_priority_max(schedule);

		int rtn = sched_setscheduler(getpid(), schedule, &param); 
		if(rtn != 0) printf("problems\n"); 
	}
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
   fprintf(logFile, line);
   fflush(logFile);
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
   fprintf(logFile, line);
   fflush(logFile);
}

