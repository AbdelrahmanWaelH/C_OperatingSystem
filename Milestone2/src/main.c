#include <semaphore.h>

typedef enum {
    READY,
    RUNNING,
    BLOCKED
} ProcessState;

typedef enum {

    FCFS,
    RR,
    MLFQ

} SchedulingPolicy;
  
typedef struct {
    int id;
    ProcessState state;
    int priority;
    int memory_start;
    int memory_end;
    int program_counter;
    // int arrival_time;
    // int time_in_queue;
    // char * instructions;
    // char * blocked_resource; 

} ProcessControlBlock; //6 words


typedef struct {

    char* name;
    char* data;

} MemoryWord;


typedef struct {

    MemoryWord memoryArray[60];
    /*
    
    6 Words PCB
    3 Words Variables
    Up to 9 Words Text/Program
    2 Free
    
    */

} Memory;

typedef struct {
    int queue[3];
} BlockedQueue;

typedef struct {
    int queue[3];
} ReadyQueue;

/*

    1. Load each process into memory (create PCB, declare variables, load program)
    2. Begin scheduling using specified algorithm.
    3. 

*/

void initMemory();
void loadProcess(char* filepath);

SchedulingPolicy policy;

char* filepathA = "";
char* filepathB = "";
char* filepathC = "";

int main(){

    // Prompt user for scheduling algo.
    initMemory();
    loadProcess(filepathA);
    loadProcess(filepathB);
    loadProcess(filepathC);

    switch (policy)
    {
    case FCFS:
        runFCFS();
        break;   
    case RR:
        runRR();
        break;    
    case MLFQ:
        runMLFQ();
        break;
    default:
        break;
    }
    
    //byebye

        


}
