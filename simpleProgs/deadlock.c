#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
int a = 0;
sem_t  S;
sem_t   Q;

void *hello1(void *arg)
{

	for(int ndx = 0;ndx<1000;ndx++){
		sem_wait(&S);
	pthread_t this = pthread_self();
	struct sched_param params;
	params.sched_priority =1;
	pthread_setschedparam(this, SCHED_FIFO, &params);
nanosleep((struct timespec[]){{0, 50000}}, NULL);
		a++;
		sem_wait(&S);	


		sem_post(&Q);
		printf("hi1 %d\n",a);
		sem_post(&S);	
	}
return NULL;

}

void *hello2(void *arg)
{
	for(int ndx = 0;ndx<1000;ndx++){
		sem_wait(&Q);
	pthread_t this = pthread_self();
	struct sched_param params;
	params.sched_priority = 2;
	pthread_setschedparam(this, SCHED_FIFO, &params);
nanosleep((struct timespec[]){{0, 50000}}, NULL);

		a--;
		sem_wait(&S);
	
		sem_post(&S);
		printf("hi2 %d\n",a);
		sem_post(&Q);
	}

	return NULL;
}


int main(int argc, char* argv[]) {
	pthread_t *threads;
	pthread_attr_t attr;
	int i;
	struct sched_param fifo_param;

	if(sem_init(&S, 0, 1)<0)
		printf("ACK");
	if(sem_init(&Q, 0, 1)<0)
		printf("ACK");
	threads=(pthread_t *)malloc(2*sizeof(*threads));
	pthread_attr_init(&attr);
	if(argc>1){
		pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
		fifo_param.sched_priority =10;	
	}
  	pthread_attr_setschedparam(&attr, &fifo_param);
	/* Start up thread */
	pthread_create(&threads[0], &attr, hello1, (void *)(1));
	if(argc>1){
		fifo_param.sched_priority =11;	
  		pthread_attr_setschedparam(&attr, &fifo_param);
	}
	pthread_create(&threads[1], &attr, hello2, (void *)(1));
	/* Synchronize the completion of each thread. */

	for (i=0; i<2; i++)
	{
		pthread_join(threads[i],NULL);
	}
	return 0;
}
