//Chung Lai Ho 101181150

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <inttypes.h>
#include "shm_com.h"

#define CONSUMER_NUM_THREADS 4
#define PRODUCER_NUM_THREADS 1

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

int RQ_turn;
int total_process_ran;

struct queue queues[CONSUMER_NUM_THREADS][3];


void *consumer_thread_function(void *arg);
void *producer_thread_function(void *arg);


int main() {
    int res;
    pthread_t a_thread[5];
    void *thread_result;
    int lots_of_threads;

    printf("\n---------------------------------------\n");
    printf("initializing Producer&Consumer Threads");
    printf("\n---------------------------------------\n");

    //Creating Producer Thread
    for(lots_of_threads = 0; lots_of_threads < PRODUCER_NUM_THREADS; lots_of_threads++) {

        res = pthread_create(&(a_thread[lots_of_threads]), NULL, producer_thread_function, (void *)&lots_of_threads);
        if (res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

    }

     pthread_join(a_thread[0], &thread_result);

    //Creating Consumer Threads
    for(lots_of_threads = 0; lots_of_threads < CONSUMER_NUM_THREADS; lots_of_threads++) {

        res = pthread_create(&(a_thread[lots_of_threads]), NULL, consumer_thread_function, (void *)&lots_of_threads);
        if (res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
        usleep(1);
    }

    printf("\n---------------------------------------\n");
    printf("Done initializing Producer&Consumer Threads");
    printf("\n---------------------------------------\n");


    printf("\n---------------------------------------\n");
    printf("Executing Processes");
    printf("\n---------------------------------------\n");


    printf("Waiting for threads to finish...\n");
    for(lots_of_threads = 5 - 1; lots_of_threads >= 0; lots_of_threads--) {
        res = pthread_join(a_thread[lots_of_threads], &thread_result);
        if (res == 0) {
            printf("Picked up a thread\n");
        }
        else {
            perror("pthread_join failed");
        }
    }


    //Total Number of Processes Ran
    printf("\n---------------------------------------\n");
    printf("Number of Processes Ran - %d", total_process_ran);
    printf("\n---------------------------------------\n");

    printf("All busy\n");
    exit(EXIT_SUCCESS);
}

//Insertion Sort Function
void insertionSort(struct process_info arr[], int n)
{
    int i, j;
    struct process_info key;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;

        while (j >= 0 && arr[j].sp > key.sp) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}


void *consumer_thread_function(void *arg) {
    int my_number = *(int *)arg;
    int quantum;
    int pri;
    int index;
    int queue_elem=5;

    int bonus;

    struct process_info null_process;

    printf("Consumer thread %d started\n", my_number);


    //Loop through all the queues RQ0, RQ1, RQ2 for all consumer threads
    //Stop the loop when all processes have run to completion
    while(queue_elem !=0){

        //Calculation for quantum or time slice
        if (queues[my_number][pri].processes[index].sp < 120){
            quantum = (140 - queues[my_number][pri].processes[index].sp)*20;
        } 
        if (queues[my_number][pri].processes[index].sp >= 120){
            quantum = (140 - queues[my_number][pri].processes[index].sp) *5;
        }

        //Update process information
        queues[my_number][pri].processes[index].time_slice.time_slice = quantum;
        queues[my_number][pri].processes[index].remain_time = queues[my_number][pri].processes[index].remain_time - quantum;
        queues[my_number][pri].processes[index].accu_time_slice = queues[my_number][pri].processes[index].accu_time_slice + quantum;
        queues[my_number][pri].processes[index].last_cpu = my_number;

        //Set up Null Process
        null_process.sp = 0;

        //"Execute" the process (500000 time delay added to see the execution process)
        if( queues[my_number][pri].processes[index].sp != 0){

            
            //For FIFO Processes
            if(strcmp(queues[my_number][pri].processes[index].process_type,"FIFO")==0){
                //FIFO Processes always run to completion
                usleep(queues[my_number][pri].processes[index].expected_execution_time + 5000);
                printf("Thread %d - Process (%s, %d) PID: %d ran to completion ✓ \n", my_number, queues[my_number][pri].processes[index].process_type, queues[my_number][pri].processes[index].sp, queues[my_number][pri].processes[index].pid);
                //Remove the completed process from the queue by substituting a "NULL" process
                queues[my_number][pri].processes[index] = null_process;
                queue_elem--;
                total_process_ran++;


            //For NORMAL Processes
            }else if(strcmp(queues[my_number][pri].processes[index].process_type,"NORMAL")==0){
                //Run Normal Process
                usleep(queues[my_number][pri].processes[index].expected_execution_time + 5000);

                //Check if simulated process execution time is less than or equal to time slice
                //Give bonus if simulated process execution time is less than or equal to time slice 
                int ran;
                ran = (rand() %10) +1;
                ran = ran * queues[my_number][pri].processes[index].time_slice.time_slice;

                if(ran < queues[my_number][pri].processes[index].time_slice.time_slice){
                    bonus = 10;
                }else if(ran == queues[my_number][pri].processes[index].time_slice.time_slice){
                    bonus = 5;
                }

                //Calculate or Update Dynamic Priority for NORMAL
                //If the process has not been run yet, the previous priority is the static priority generated by the producer
                if(queues[my_number][pri].processes[index].first_pri == 0){
                    queues[my_number][pri].processes[index].dp = max(100, min(queues[my_number][pri].processes[index].sp - bonus+5, 139));
                    queues[my_number][pri].processes[index].first_pri = 1;
                }else{
                //If the process has been run, the previous priority is the previous dynamic priority
                    queues[my_number][pri].processes[index].dp = max(100, min(queues[my_number][pri].processes[index].dp - bonus+5, 139));
                }

                //Run to completion
                printf("Thread %d - Process (%s, %d) PID: %d ran to completion ✓ \n", my_number, queues[my_number][pri].processes[index].process_type, queues[my_number][pri].processes[index].sp, queues[my_number][pri].processes[index].pid);
                //Remove the completed process from the queue by substituting a "NULL" process
                queues[my_number][pri].processes[index] = null_process;
                queue_elem--;
                total_process_ran++;
                


            //For RR Processes 
            }else if(strcmp(queues[my_number][pri].processes[index].process_type,"RR")==0){
                //Run RR Process, uses entire time slice 
                usleep(queues[my_number][pri].processes[index].time_slice.time_slice + 5000);
                //If there is execution time left after running process, put it back in queue based on its priority
                if(queues[my_number][pri].processes[index].expected_execution_time - queues[my_number][pri].processes[index].time_slice.time_slice > 0){
                    //Recalculate the time slice for the process
                    queues[my_number][pri].processes[index].expected_execution_time = queues[my_number][pri].processes[index].expected_execution_time - queues[my_number][pri].processes[index].time_slice.time_slice;
                    printf("Thread %d - Process (%s, %d) PID: %d put back into queue ✖ \n", my_number, queues[my_number][pri].processes[index].process_type, queues[my_number][pri].processes[index].sp, queues[my_number][pri].processes[index].pid);
                }else{
                //Run to completion, if there is no execution time left
                    
                    printf("Thread %d - Process (%s, %d) PID: %d ran to completion ✓ \n", my_number, queues[my_number][pri].processes[index].process_type, queues[my_number][pri].processes[index].sp, queues[my_number][pri].processes[index].pid);
                    //Remove the completed process from the queue by substituting a "NULL" process
                    queues[my_number][pri].processes[index] = null_process;
                    queue_elem--;
                    total_process_ran++;
                }
                
            }

            //Re-arrange the associated queue based on priorities everytime a process is ran
            insertionSort(queues[my_number][pri].processes, 5);
        }

        index++;

        //Index to which process in queue to execute
        if(index == 5){
            index = index%5;
            pri++;
        }

        //Index to which priority queue to select process from
        if(pri == 3){
            pri = pri %3;
        }
        
        //Print consumer thread done when no processes left in queue
        if(queue_elem==0){
            printf("\nThread %d have run all processes\n", my_number);
        }
        
    }
   
    pthread_exit(NULL);
}



void *producer_thread_function(void *arg) {
    struct process_info process;
    int my_number = *(int *)arg;
    char input_data[1024] = "NORMAL,120,6100,NORMAL,100,4000,FIFO,40,5800,RR,33,9700,NORMAL,130,6700,NORMAL,115,5500,FIFO,70,4000,FIFO,25,2900,RR,35,6800,FIFO,50,7200,RR,48,8000,NORMAL,135,5300,NORMAL,109,2900,NORMAL,107,6800,RR,22,5500,NORMAL,115,5000,NORMAL,108,7300,NORMAL,100,3200,NORMAL,116,7400,NORMAL,123,3300";
    char sched[128];

   const char s[2] = ",";
   char *token;

    int i=0;
    int pri=0;
    int turn =0;
    int pid=0;

    int count=0;

    //Keep track of the index for each RQ
    int pri0_0_count=0;
    int pri0_1_count=0;
    int pri0_2_count=0;

    int pri1_0_count=0;
    int pri1_1_count=0;
    int pri1_2_count=0;

    int pri2_0_count=0;
    int pri2_1_count=0;
    int pri2_2_count=0;

    int pri3_0_count=0;
    int pri3_1_count=0;
    int pri3_2_count=0;
   
   printf("Producer thread %d started\n", my_number);
   
   token = strtok(input_data, s);
   
   //Split the input data and store them into a process and distrubte them to appropriate queues
   while(token!= NULL){
    //When turn is 0, set PID and set Process type
   	if(turn == 0){
   		process.pid = pid;
   		pid++;
   		strcpy(process.process_type, token);  	
    //When turn is 1, set static priority	
   	}else if(turn ==1){
   		process.sp = atoi(token);	
    //When turn is 2, set execution time and put the process in appropriate queues based on the static priority
   	}else if(turn ==2){
   		process.expected_execution_time = atoi(token);
        //Put to RQ0 if static priorirt < 100
        if(process.sp < 100){
            pri = 0;
            if(i==0){
                count = pri0_0_count;
                pri0_0_count++;
            }else if(i==1){
                count = pri1_0_count;
                pri1_0_count++;
            }else if(i==2){
                count = pri2_0_count;
                pri2_0_count++;
            }else if(i==3){
                count = pri3_0_count;
                pri3_0_count++;
            }
        //Put to RQ1 if static priorirt >= 100, < 130
   	    }else if(process.sp >= 100 && process.sp < 130){
            pri = 1;
            if(i==0){
                count = pri0_1_count;
                pri0_1_count++;
            }else if(i==1){
                count = pri1_1_count;
                pri1_1_count++;
            }else if(i==2){
                count = pri2_1_count;
                pri2_1_count++;
            }else if(i==3){
                count = pri3_1_count;
                pri3_1_count++;
            }
        //Put to RQ2 if static priorirt >= 130, <140
   	    }else if(process.sp >= 130 && process.sp < 140){
            pri = 2;
            if(i==0){
                count = pri0_2_count;
                pri0_2_count++;
            }else if(i==1){
                count = pri1_2_count;
                pri1_2_count++;
            }else if(i==2){
                count = pri2_2_count;
                pri2_2_count++;
            }else if(i==3){
                count = pri3_2_count;
                pri3_2_count++;
            }
        }

        //Put process in queue
        queues[i][pri].processes[count] = process;
        
   	}
   	token = strtok(NULL, s);
   	turn++;
   	turn = turn%3;

    i++;
    i = i%4;

    count++;
    count = count%5;
   } 

    //Insertion Sort all the queues
    for(int i=0; i<4; i++){
        for(int j=0; j<3; j++){
            insertionSort(queues[i][j].processes ,5);
        }
    }

    pthread_exit(NULL);
}
