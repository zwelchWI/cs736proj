#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>

#define SLEEP 30 
int a = 0;
sem_t  S;
sem_t   Q;
int flag = 0;
//taken from pthreads man page, prints out the current priority and
//the policy
void print_sched_attr(){
	int policy, rtn; 
	struct sched_param param; 
	
	rtn = pthread_getschedparam(pthread_self(), &policy, &param); 
	if(rtn != 0) perror("Unable to retrieve pthread sched param: "); 

	printf("Scheduler attributes for pid %d, thread %ld, : policy: %s, priority %d\n",
		getpid(),  
		syscall(SYS_gettid), 
		(policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                (policy == SCHED_RR)    ? "SCHED_RR" :
                (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                "???",
                param.sched_priority);
}

void *hello1(void *arg)
{

	for(int ndx = 0;ndx<10000;ndx++){
		sem_wait(&S);


pthread_t this = pthread_self();
  struct sched_param fifo_param;

if(flag){
                fifo_param.sched_priority =sched_get_priority_min(SCHED_RR)+
                                                        rand()%sched_get_priority_max(SCHED_RR);
                if(pthread_setschedparam(this,SCHED_RR,&fifo_param) != 0)
			perror("couldn't set sched:");
		print_sched_attr(); 
		sleep(SLEEP); 
usleep(1);
}
		sem_wait(&Q);	
if(flag){       

                fifo_param.sched_priority =sched_get_priority_min(SCHED_RR)+
                                                        rand()%sched_get_priority_max(SCHED_RR);
                if(pthread_setschedparam(this,SCHED_RR,&fifo_param) != 0)
			perror("couldn't set sched:"); 
usleep(1);
}
                printf("no\n");

		a++;
	
		
		printf("hi1 %d\n",a);
		sem_post(&Q);
if(flag){       

                fifo_param.sched_priority =sched_get_priority_min(SCHED_RR)+
                                                        rand()%sched_get_priority_max(SCHED_RR);
                if(pthread_setschedparam(this,SCHED_RR,&fifo_param) != 0)
			perror("couldn't set sched:");
usleep(1);
}
		sem_post(&S);
if(flag){       

                fifo_param.sched_priority =sched_get_priority_min(SCHED_RR)+
                                                        rand()%sched_get_priority_max(SCHED_RR);
                if(pthread_setschedparam(this,SCHED_RR,&fifo_param) != 0)
			perror("couldn't set sched:");
usleep(1);
}
	}
return NULL;

}

void *hello2(void *arg)
{

	for(int ndx = 0;ndx<10000;ndx++){
		sem_wait(&Q);
 struct sched_param fifo_param;// = pthread_self();

		pthread_t this = pthread_self();
if(flag){       


		fifo_param.sched_priority =sched_get_priority_min(SCHED_RR)+
                                                        rand()%sched_get_priority_max(SCHED_RR);
                if(pthread_setschedparam(this,SCHED_RR,&fifo_param) != 0)
			perror("couldn't set sched:");
usleep(1);
		print_sched_attr(); 
		sleep(SLEEP); 
}
		sem_wait(&S);
if(flag){       

                fifo_param.sched_priority =sched_get_priority_min(SCHED_RR)+
                                                        rand()%sched_get_priority_max(SCHED_RR);
                if(pthread_setschedparam(this,SCHED_RR,&fifo_param) != 0)
			perror("couldn't set sched:");
usleep(1);
}
                printf("no\n");

                printf("hi2 %d\n",a);
	   	//a--;// printf("hi2 %d\n",a);
		sem_post(&S);
if(flag){       
                fifo_param.sched_priority =sched_get_priority_min(SCHED_RR)+
                                                        rand()%sched_get_priority_max(SCHED_RR);
                if(pthread_setschedparam(this,SCHED_RR,&fifo_param) != 0)
			perror("couldn't set sched:");
usleep(1);
}		sem_post(&Q);
 if(flag){       
               fifo_param.sched_priority =sched_get_priority_min(SCHED_RR)+
                                                        rand()%sched_get_priority_max(SCHED_RR);
                if(pthread_setschedparam(this,SCHED_RR,&fifo_param) != 0)
			perror("couldn't set sched:");
usleep(1);
}	}

	return NULL;
}


int main(int argc, char* argv[]) {
	pthread_t *threads;
	pthread_attr_t attr;
	struct sched_param fifo_param;

        pthread_attr_t attr2;
        struct sched_param fifo_param2;


	if(sem_init(&S, 0, 1)<0)
		printf("ACK");
	if(sem_init(&Q, 0, 1)<0)
		printf("ACK");
	threads=(pthread_t *)malloc(2*sizeof(*threads));
	int ret = pthread_attr_init(&attr);
                if(ret < 0){
                        printf("RETURN VALUE FAIL\n");
                        return 0;
                }
	ret = pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
                if(ret < 0){
                        printf("RETURN VALUE FAIL\n");
                        return 0;
                }

        ret = pthread_attr_init(&attr2);
                if(ret < 0){
                        printf("RETURN VALUE FAIL\n");
                        return 0;
                }
        ret = pthread_attr_setinheritsched(&attr2,PTHREAD_EXPLICIT_SCHED);
                if(ret < 0){
                        printf("RETURN VALUE FAIL\n");
                        return 0;
                }

	if(argc>2){
		flag = 1;
		printf("Using alternate scheduler, pid of parent: %d\n", getpid());	
		ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
		if(ret < 0){
			printf("RETURN VALUE FAIL\n");
			return 0;	
		}
		fifo_param.sched_priority =sched_get_priority_min(SCHED_RR);	
	}

	ret = pthread_attr_setschedparam(&attr, &fifo_param);
       	if(ret<0){
                printf("FAILEDED\n");
                return 0;
        }
/* Start up thread */
	 ret = pthread_create(&threads[0], &attr, hello1, (void *)(1));
	if(ret<0){
		printf("FAILEDED\n");
		return 0;
	}
	if(argc>2){
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

	}
	ret = pthread_create(&threads[1], &attr2, hello2, (void *)(1));
	if(ret<0){
                printf("FAILEDED\n");
                return 0;
        }
/* Synchronize the completion of each thread. */

	printf("DONE\n");
		ret = pthread_join(threads[1],NULL);
                ret = pthread_join(threads[0],NULL);

	return 0;
}
