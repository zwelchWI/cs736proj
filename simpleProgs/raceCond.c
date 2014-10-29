#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
int a = 0;
int flag=0;
void *hello1(void *arg)
{
	for(int ndx = 0;ndx<10000000;ndx++){
		a++;
		if(flag){
			printf("hi1 %d\n",a);
			flag=0;
		}
	}
	return NULL;
}

void *hello2(void *arg)
{
	for(int ndx = 0;ndx<10000000;ndx++){
		a--;
		if(!flag){
			printf("hi2 %d\n",a);
			flag=1;
		}
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
	if(argc>2){
		int ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
		if(ret < 0){
			printf("RETURN VALUE FAIL\n");
			return 0;	
	}
		fifo_param.sched_priority =15;	
pthread_attr_setschedparam(&attr, &fifo_param);	
}
	/* Start up thread */
	pthread_create(&threads[0], &attr, hello1, (void *)(1));
	if(argc>2){
		fifo_param.sched_priority =10;	
  		pthread_attr_setschedparam(&attr, &fifo_param);
	}

	pthread_create(&threads[1], &attr, hello2, (void *)(1));
	/* Synchronize the completion of each thread. */

	for (i=0; i<2; i++){
		pthread_join(threads[i],NULL);
	}
	return 0;
}
