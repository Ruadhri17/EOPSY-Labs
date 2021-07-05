#include <stdio.h>
#include <stdbool.h>  // boolean values
#include <signal.h>   // signal(), kill()
#include <unistd.h>   // fork()
#include <sys/wait.h> // wait()
#include <stdlib.h>   // exit()

#define NUM_CHILD 6
#ifdef WITH_SIGNALS // Things needed to 2. Task
bool interrupt_flag = false;
void signal_handler(int signum);
#endif

int main()
{
#ifdef WITH_SIGNALS
    // As signal() should not be used for ignoring of all signals, sigaction() is used
    struct sigaction old_handlers[NSIG]; // starting (default) handlers for signals
    struct sigaction change_handlers;    // edit handlers 
    
    change_handlers.sa_handler = SIG_IGN;
    change_handlers.sa_flags = 0;
    sigemptyset(&change_handlers.sa_mask);

    for (int sig_num = 1; sig_num < NSIG; sig_num++) //Ignore All signals
        sigaction(sig_num, &change_handlers, &old_handlers[sig_num-1]);

    sigaction(SIGCHLD, SIG_DFL, NULL);       // Default handler for Child
    sigaction(SIGINT, signal_handler, NULL); // Own handler for keyboard interruption
#endif

    pid_t child_id_array[NUM_CHILD];   // ID array for all created child processes
    pid_t creation_id;                 // ID for analizing during creation of process
    pid_t termination_id;              // ID for analizing during termination of process
    int child_termination_counter = 0; // number of terminated child processes

    for (int i = 0; i < NUM_CHILD; i++)
    {
        creation_id = fork(); // create new process, save the id
        child_id_array[i] = creation_id;

        if (creation_id == 0)
        {
#ifdef WITH_SIGNALS
            signal(SIGINT, SIG_IGN);         // ignore standard keyboard interrupt
            signal(SIGTERM, signal_handler); // own handler for termination due to keyboard interruption
#endif
            printf("CHILD[%d]: CREATED BY PARENT[%d]\n", getpid(), getppid());
            sleep(10);
            printf("CHILD[%d]: EXECUTION FINISHED\n", getpid());
            exit(0);
        }
        if (creation_id < 0) // In case of failing in creation of new process, delete all created processes, send message and exit
        {
            printf("PARENT[%d]: FAILED TO CREATE #%d CHILD\n", getpid(), i);
            for (int j = 0; j < i; j++)
            {
                kill(child_id_array[j], SIGTERM);
                sleep(1);
            }
            exit(1);
        }
        if (i != NUM_CHILD)
            sleep(1); // wait a second before next creation of process
#ifdef WITH_SIGNALS
        if (interrupt_flag == true) //between creation of two processes check if keyboard interruption occured
        {
            printf("PARENT[%d]: NEW PROCESS CREATION IS CANCELED\n", getpid());
            printf("PARENT[%d]: TERMINATING ALL CREATED CHILD PROCESSES\n", getpid());
            sleep(1);
            for (int j = 0; j <= i; j++)
            {
                kill(child_id_array[j], SIGTERM); //send SIGTERM to all already created child processes
                sleep(1);
            }
            break;
        }
#endif
    }
#ifdef WITH_SIGNALS
    if (interrupt_flag == false) //if there were no interruption, print that all child created successfully
#endif
        printf("PARENT[%d]: ALL CHILD PROCESSES CREATED SUCCESSFULLY!\n", getpid());
    while (true)
    {
        if ((termination_id = wait(NULL)) == -1) //if all child processes were terminated send message and print number of terminated processes
        {
            printf("PARENT[%d]: ALL CHILD PROCESEES WERE TERMINATED!\n", getpid());
            printf("PARENT[%d]: NUMBER OF TERMINATED CHILD PROCESSES: %d\n", getpid(), child_termination_counter);
            break;
        }
        else
            child_termination_counter++;
    }
#ifdef WITH_SIGNALS
    for (int sig_num = 1; sig_num < NSIG; sig_num++) //Restore All signals
        sigaction(sig_num, &old_handlers[sig_num -1], NULL);
#endif
    exit(0);
}
#ifdef WITH_SIGNALS
void signal_handler(int signum)
{
    if (signum == SIGINT) //if keyboard interruption occured, set flag and print message
    {
        printf("\nPARENT[%d]: RECIVED KEYBOARD INTERRUPT SIGNAL\n", getpid());
        interrupt_flag = true;
        sleep(1);
    }
    if (signum == SIGTERM) // if SIGTERM occured after keyboard interruption, terminate child processes
    {
        printf("CHILD[%d]: PROCESS TERMINATED BY PARENT[%d]\n", getpid(), getppid());
        exit(1);
    }
}
#endif
