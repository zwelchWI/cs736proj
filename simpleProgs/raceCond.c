#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
int a = 0;
void *hello1(void *arg)
{
	for(int ndx = 0;ndx<1000;ndx++){
		a++;
		printf("hi1 %d\n",a);
	}
	return NULL;
}

void *hello2(void *arg)
{
	for(int ndx = 0;ndx<1000;ndx++){
		a--;
		printf("hi2 %d\n",a);
	}
	return NULL;
}


int main(int argc, char* argv[]) {
	pthread_t *threads;
	pthread_attr_t attr;
	int i;
	struct sched_param fifo_param;

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
