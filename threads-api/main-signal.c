#include <stdio.h>

#include "common_threads.h"

int done = 0;

void* worker(void* arg) {
    printf("this should print first\n");
    done = 1;
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p;
    Pthread_create(&p, NULL, worker, NULL);
    while (done == 0)//Busy-waiting burns CPU cycles: The loop repeatedly reads done as fast as the CPU allows,
    // using 100% of a core while waiting. 
    // That wastes CPU time and power that could be used for useful work or to let other processes ru
	;
    printf("this should print last\n");
    return 0;
}
