#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
int main(int argc, char* argv[]){
	printf("Pid: %d, Should I start? ", getpid()); 
	while(getchar() != 'y'){
		usleep(100); 
	}
	int i;
	for(i = 0; i < 10; i++){
		int pid = fork();
		//child proc
		if(pid == 0){
			printf("I'm child %d.\n", getpid()); 
			//this will give us current priority
			system("chrt -p $$"); 
			int j;
			for(j = 0; j < INT_MAX; j++){
				if(j == INT_MAX/2){
					printf("child %d\n half way done", getpid());
				}
			}
			_exit(0); 
		}
	}
	int wpid;
	while((wpid = wait(NULL)) > 0); 
	return 1;
}
			
