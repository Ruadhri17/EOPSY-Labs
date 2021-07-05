/* ANSWERS TO QUESTIONS BELOW THE CODE*/

#include <stdio.h>
#include <stdbool.h>  
#include <unistd.h> 
#include <stdlib.h>  
#include <pthread.h>

#define N 5 // number of philosophers
#define ITERATIONS 10 // num of iterations (to make program finite)
#define LEFT (i + N - 1) % N // id of left philo from current philo perspective
#define RIGHT (i + 1) % N // id of right philo from current philo perspective

// philosophers' states 
#define THINKING	0
#define HUNGRY		1
#define EATING		2

pthread_mutex_t grab_mutex = { 1 }; 
pthread_mutex_t philo_mutexes[N] = { 0 };
pthread_t philosophers[N];
int philo_states[N] = { THINKING }; // by deafult philos are set to think

void test(int i) // test if philo can start eating
{
    if (philo_states[i] == HUNGRY && philo_states[LEFT] != EATING && philo_states[RIGHT] != EATING) //if hungry and neighbors does not eat, let philo eat
        {
            //mark as eating
            philo_states[i] = EATING;
            sleep(1);
            //signal for availability
            pthread_mutex_unlock(&philo_mutexes[i]);
        }
}

void grab_forks(int i) 
{
    pthread_mutex_lock(&grab_mutex);
    
    //change state for hungry
    philo_states[i] = HUNGRY;
    test(i);
    
    pthread_mutex_unlock(&grab_mutex); 
    //reset lock
    pthread_mutex_lock(&philo_mutexes[i]);
    
}

void put_away_forks(int i)
{
    
    pthread_mutex_lock(&grab_mutex);
    
    //start thinking
    philo_states[i] = THINKING;
    
    
    //let neighbour eat if they can
    test(LEFT);
    test(RIGHT);
    
    pthread_mutex_unlock(&grab_mutex);
    
}
void* simulate_philosopher(void *arg)
{
    //set philo id
    int i = *(int*)arg;
    printf("PHILOSOPHER #%d: GATHERED AROUND THE TABLE!\n", i);
    sleep(4); // wait for all philos
    //simulate philo routine
    for (int j = 0; j < ITERATIONS; j++) {
        grab_forks(i);
        printf("PHILOSOPHER #%d: EATING\n", i);
		sleep(1);
		put_away_forks(i);
        printf("PHILOSOPHER #%d: STOPPED EATING\n", i);
        sleep(1);
	}
    printf("PHILOSOPHER #%d: LEFT TABLE\n", i);
}
int main() 
{
    // initialize grab mutex
    pthread_mutex_init(&grab_mutex, NULL);
    pthread_mutex_unlock(&grab_mutex);

    // initialize philo mutexes 
    for (int i = 0; i < N; i++) 
    {
        pthread_mutex_init(&philo_mutexes[i], NULL);
		pthread_mutex_lock(&philo_mutexes[i]);
	}

    // create philo threads and pass theirs id
    for (int i = 0; i < N; i++){
        pthread_create(&philosophers[i], NULL, simulate_philosopher, (void*)&i);
        sleep(1);
    }
    // wait for them to finish
    for (int i = 0; i < N; i++)
        pthread_join(philosophers[i], NULL);
    
    // destroy philos and grab mutex
    pthread_mutex_destroy(&grab_mutex);
    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&philo_mutexes[i]);

    exit(0);
}

/* 
Question 1
It that case it would not be sufficient. Mutex is locking mechanism and semaphore is signaling mechanism.
Mutex could potentially block other threads to enter critical section causing program to stuck

Question 2
M mutex is set to 1 in order to lock it, let it perform some operation while preventing other processes to try do it simultaneously.
When we will unlock it we allow other threads to do their task.
Array s is set to 0 to let it inform about eating state. (And if other can eat or not)
*/
