/*
Author: Jason England
Date Due: 11/29/2022
Purpose: This program creates 7 threads each representing a philosopher,
		and each thread will think, take forks from the table, eat,
		and put forks back on the table 20 times. The philosophers cannot
		take forks that are currently being used by another one.
To compile: gcc -pthread JasonEngland_philosophers.c
To run: ./a.out
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//defining constants
#define PHILOSOPHERS 7
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT (i+PHILOSOPHERS-1)%PHILOSOPHERS
#define RIGHT (i+1)%PHILOSOPHERS

//Creating main mutex
pthread_mutex_t mutex;
//Creating array for the states of philosophers
int state[PHILOSOPHERS];
//Creating an array of mutexes, one for each philosopher
pthread_mutex_t s[PHILOSOPHERS];


/*This function will test if a philosopher 'i' is currently hungry and neither of
	its neighbors are currently eating. If this is true, the philosopher will pick up
	the forks and then unlock the associated mutex within the mutex array.*/
void* test(int i){
	if(state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING){
		//Sets state of philosopher to Eating
		state[i] = EATING;
		int status;

		//Outputting messages for picking up forks into the text file
		char command[100];
		sprintf(command, "echo 'Philosopher #%d picks up fork %d\n' >> philosopher_output.txt", i, i);
		status = system(command);
		sprintf(command, "echo 'Philosopher #%d picks up fork %d\n' >> philosopher_output.txt", i, RIGHT);
		status = system(command);

		//Printing to console
		printf("Philosopher #%d picks up fork %d\n", i, i);
		printf("Philosopher #%d picks up fork %d\n", i, RIGHT);

		//Unlocking that philosophers mutex
		pthread_mutex_unlock(&s[i]);
	}
}

/*This function will have the philosopher 'tid' output a message saying that it is eating*/
void* eating(void* tid){
	int status;
	int i = (intptr_t)tid;

	//Outputting message to text file saying that the philosopher is eating
	printf("Philosopher #%d is Eating\n", i);
	char command[100];
	sprintf(command, "echo 'philosopher #%d is eating\n' >> philosopher_output.txt", i);
	status = system(command);
	printf("Philosopher #%d is done Eating\n", i);
}

/*This function will have the philosopher 'myid' begin to think by locking the main mutex,
	outputting the thinking message, then relinquishing the main mutex*/
void * thinking(void * myid)
{
	int status;

	//converts myid to an int called tid
	int tid = (intptr_t)myid;

	//create a critical section
	pthread_mutex_lock(&mutex);

	//Sets the philosophers state to Thinking
	state[tid] = THINKING;
	
	/* This function prints the thread’s identifier and then exits. */
	printf("Philosopher #%d is thinking\n", tid);

	/*create a bash command to run*/
	char command[100];
	sprintf(command, "echo 'philosopher #%d is thinking\n' >> philosopher_output.txt", tid);

	/*run the command*/
	status = system(command);

	/*print a farewell message*/
	printf("Philosopher #%d is done thinking\n", tid);
	
	//leave the critical section
	pthread_mutex_unlock(&mutex);
}


/*This function will have the philosopher 'myid' attempt to take the permitted forks by
	calling the test() function. If the philosopher is unable to take the forks then it is blocked.*/
void* take_Forks(void * myid)
{
	int status;
	int i = (intptr_t)myid;

	//Locking main mutex
	pthread_mutex_lock(&mutex);

	//Setting state to hungry so we know it's waiting for forks
	state[i] = HUNGRY;
	
	//Printing to console
	printf("Philosopher #%d is HUNGRY\n", i);
	
	//Checks to see if the philosopher can actually take the forks
	test(i);

	//Unlocks main mutex and locks that philospohers mutex
	pthread_mutex_unlock(&mutex);
	//This will block this thread from continuing until it can get the forks required
	pthread_mutex_lock(&s[i]);
	
}

/*This function has the philosopher 'myid' put the forks back on the table
	and set its state back to thinking. It will then check if that philosopher's
	neighbors are waiting for forks.*/
void* put_Forks(void * myid)
{
	int status;
	int i = (intptr_t)myid;

	//Locks main mutex
	pthread_mutex_lock(&mutex);
	
	//Outputting messages for putting down forks to text file
	char command[100];
	sprintf(command, "echo 'Philosopher #%d puts down fork %d\n' >> philosopher_output.txt", i, i);
	status = system(command);
	sprintf(command, "echo 'Philosopher #%d puts down fork %d\n' >> philosopher_output.txt", i, RIGHT);
	status = system(command);

	//Prints to console
	printf("Philosopher #%d is done EATING and is now THINKING\n", i);
	printf("Philosopher #%d puts down fork %d\n", i, i);
	printf("Philosopher #%d puts down fork %d\n", i, RIGHT);

	//Sets state of philosopher back to thinking
	state[i] = THINKING;

	//checks if philosophers to the left or right are waiting to eat and can grab forks
	test(LEFT);
	test(RIGHT);

	//unlocks main mutex
	pthread_mutex_unlock(&mutex);
}

/*This function is called by each philosopher thread when it is created and calls all previous functions.
	This function loops 20 times for each philosopher.*/
void* philosopher(void* i)
{
	//Loops 20 times
    for(int j = 0; j < 20; j++) {
		//Makes the philosopher think
		thinking(i);
 
		//Makes the philosopher take forks if available
        take_Forks(i);

		//Makes the philosopher eat oonce they have taken forks
		eating(i);

		//Makes the philosopher put the forks back on the table once they are done eating
        put_Forks(i);
    }
	//Terminates the thread after the loop
	pthread_exit(NULL);
}

/*Main function which creates the necessary mutexes and threads*/
int main(int argc, char * argv[])
{
	pthread_t threads[PHILOSOPHERS];
	int status, i;

	//create a mutex
		if (pthread_mutex_init(&mutex, NULL) != 0)
		{
			printf("\n mutex init failed\n");
			return 1;
		}

	//Creates the arrays for each philosopher and starts them locked
	for(i=0; i < PHILOSOPHERS; i++) {
		if (pthread_mutex_init(&s[i], NULL) != 0)
		{
			printf("\n mutex init failed\n");
			return 1;
		}
		pthread_mutex_lock(&s[i]);
	}
	//count through the amount of threads for each philosopher
	for(i=0; i < PHILOSOPHERS; i++) {
		//run main thread
		printf("Main here. Creating philosopher thread %d\n", i);
		//create the threads for the philosophers
		status = pthread_create(&threads[i], NULL, philosopher, (void *)i);
		//check for errors with creating threads
		if (status != 0) {
			printf("Oops. pthread create returned error code %d\n", status);
			exit(-1);
		}
	}
	//have the main thread wait for all the threads created
	for (int j = 0; j < PHILOSOPHERS; j++)
	{
		pthread_join (threads[j], NULL);
	}
	//deletes the mutexs
	pthread_mutex_destroy(&mutex);
	for(i=0; i < PHILOSOPHERS; i++) {
		pthread_mutex_destroy(&s[i]);
	}
	exit(0);
}
