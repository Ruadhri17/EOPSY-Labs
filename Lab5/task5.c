#include <stdio.h>
#include <stdbool.h>  
#include <unistd.h> 
#include <stdlib.h>  
#include <sys/wait.h>    
#include <sys/sem.h> 
#include <sys/ipc.h>
#include <sys/types.h>

#define NUM_OF_PHILOSOPHERS 5 // number of philosophers
#define NUM_OF_ITERATIONS 5 // number of iterations

int forks, talk;
struct sembuf operations; // used to perfrom samaphore operations

void grab_forks(int left_fork_id) // function  for grabbing both forks by one of the 
{
    // set flag to default val. 0 and calc. id value of right fork
    operations.sem_flg = 0;
    int right_fork_id = ((left_fork_id + 1) % NUM_OF_PHILOSOPHERS);
    // grab first fork
    operations.sem_op = -1;
    operations.sem_num = left_fork_id;
    semop(forks, &operations, 1);
    // grab second fork
    operations.sem_op = -1;
    operations.sem_num = right_fork_id;
    semop(forks, &operations, 1);
    
    printf("FORK #%d AND #%d GRABBED\n", left_fork_id, right_fork_id);
    return;
}
void put_away_forks(int left_fork_id)
{
    // set flag to default val. 0 and calc. id value of right fork
    operations.sem_flg = 0;
    int right_fork_id = ((left_fork_id + 1) % NUM_OF_PHILOSOPHERS);
    // put away first fork
    operations.sem_op = +1;
    operations.sem_num = left_fork_id;
    semop(forks, &operations, 1);
    // put away second fork
    operations.sem_op = +1;
    operations.sem_num = right_fork_id;
    semop(forks, &operations, 1);
    
    printf("FORK #%d AND #%d PUT AWAY\n", left_fork_id, right_fork_id);
    return;
}

void simulate_philosopher(int number_of_philosopher)
{
    int left_fork; // id of left fork 
    operations.sem_flg = 0; // set flag to 0 (wont change during this program)

    if(number_of_philosopher < NUM_OF_PHILOSOPHERS) //adjust proper fork to philo
        left_fork = number_of_philosopher;
    else
        left_fork = 0;
    // go to table
    operations.sem_op = -1;
    operations.sem_num = 0;
    semop(talk, &operations, 1);
    printf("PHILOSOPHER #%d: GATHERED AROUND THE TABLE!\n", number_of_philosopher);
    // wait until every philo will sit at the table
    operations.sem_op = 0;
    operations.sem_num = 0;
    semop(talk, &operations, 1);

    sleep(1);
    // loop for thinking/eating cycles of each philo
    for(int i = 0; i < NUM_OF_ITERATIONS; i++)
    {
        printf("PHILOSOPHER %d: ", number_of_philosopher);
        grab_forks(left_fork);
        printf("PHILOSOPHER %d: EATING\n", number_of_philosopher);
        sleep(1);
        
        printf("PHILOSOPHER %d: ", number_of_philosopher);
        put_away_forks(left_fork);
        printf("PHILOSOPHER %d: FINISHED EATING\n", number_of_philosopher);
        sleep(1);
    }
    printf("PHILOSOPHER #%d: LEAVE THE TABLE!\n", number_of_philosopher);
    return;
}
int main() 
{
    pid_t philosophers_array[NUM_OF_PHILOSOPHERS];
    pid_t creation_id, termination_id;
    // allocate forks; set value of semaphhore to 1; 5 forks in total; if error, exit
    forks = semget(IPC_PRIVATE, NUM_OF_PHILOSOPHERS, IPC_CREAT | 0600);
    if (forks == -1)
        exit(1);
    for (int i = 0;  i < NUM_OF_PHILOSOPHERS; i++)
        semctl(forks, i, SETVAL, 1);
    // allocate talk; main use to prevent philo from eating at the start
    talk = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (talk == -1)
        exit(1);
    semctl(talk, 0, SETVAL, 5);
    
    for (int i = 0; i < NUM_OF_PHILOSOPHERS; i++)
    {
        creation_id = fork(); // create philo process
        philosophers_array[i] = creation_id;
        if (creation_id == 0)
        {
            printf("PHILOSOPHER #%d: CREATED\n", i);
            simulate_philosopher(i); // go to function which simulates behavior of philo
            exit(0); // then end philo process
        }
        if (creation_id < 0) // if failed in creation of any philo, stop every one already created and exit
        {
            printf("PHILOSOPHER #%d: FAILED TO CREATE\n", getpid(), i);
            for (int j = 0; j <= i; j++)
            {
                kill(philosophers_array[j], SIGTERM); 
                sleep(1);
            }
            exit(1);
        }
        sleep(1);
    }
    while (true) // wait for all philosophers to finish their meal
    {
        if ((termination_id = wait(NULL)) == -1) 
        {
            printf("ALL PHILOSOPHERS LEFT THE TABLE!\n");
            break;
        }
    }
    // clear after every created semaphore
    semctl(forks, 0, IPC_RMID);
    semctl(talk, 0, IPC_RMID);

    exit(0);
}