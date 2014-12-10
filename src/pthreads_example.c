/* From http://timmurphy.org/2010/05/04/pthreads-in-c-a-minimal-working-example/ */

#include <pthread.h>
#include <stdio.h>

/* this function is run by the second thread */
void *inc_x(void *x_void_ptr)
{

	/* increment x to 100 */
	int *x_ptr = (int *)x_void_ptr;
	while(++(*x_ptr) < 100);

	printf("x increment finished\n");

	/* the function must return something - NULL will do */
	return NULL;

}

int main()
{

	int x = 0, y = 0;

	/* this variable is our reference to the second thread */
	pthread_t inc_x_thread;

	/* show the initial values of x and y */
	printf("x: %d, y: %d\n", x, y);

	/* create a second thread which executes inc_x(&x) */
	if(pthread_create(&inc_x_thread, NULL, inc_x, &x)) {

		fprintf(stderr, "Error creating thread\n");
		return 1;

	}
	/* increment y to 100 in the first thread */
	while(++y < 100);

	printf("y increment finished\n");

	/* wait for the second thread to finish */
	if(pthread_join(inc_x_thread, NULL)) {

		fprintf(stderr, "Error joining thread\n");
		return 2;

	}

	/* show the results - x is now 100 thanks to the second thread */
	printf("x: %d, y: %d\n", x, y);

	return 0;

}
