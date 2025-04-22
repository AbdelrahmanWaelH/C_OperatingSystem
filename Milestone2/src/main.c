#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/Queue.h"
#include "../include/FileReader.h"

#define CODE_SEGMENT_OFFSET 9

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
    Queue blockedQueue;
} BlockedQueue;

typedef struct {
    Queue readyQueue;
} ReadyQueue;

/*

    1. Load each process into memory (create PCB, declare variables, load program)
    2. Begin scheduling using specified algorithm.
    3. 

*/



void removeNewline(char* buffer) {
    // Find the newline character (if any) and replace it with '\0'
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    }
}



void initMemory();
void initPCBs();
void initSemaphores();
void initQueues();
char* processToString(ProcessState state);
void displayMemory(Memory* mainMemory);

/*Safer posting for semaphore to avoid overflow/underflow*/
void safe_sem_post(sem_t* __sem);

/*Safer wait for semaphore to avoid overflow/underflow*/
void safe_sem_wait(sem_t* __sem);

ProcessControlBlock loadProcess(char* filepath);
void executeSingleLinePCB(ProcessControlBlock* processControlBlock);
char** lineParser(char* line);

SchedulingPolicy policy;

sem_t userInputSemaphore;
sem_t userOutputSemaphore;
sem_t fileSemaphore;

int processIdCounter = 0;

Memory mainMemory;

char* filepathA =  "Program_1.txt";
char* filepathB = "";
char* filepathC = "";

int main(){

    // Prompt user for scheduling algo.
    initMemory();
    ProcessControlBlock pcbA = loadProcess(filepathA);
    displayMemory(&mainMemory);

    executeSingleLinePCB(&pcbA);

    displayMemory(&mainMemory);

    // loadProcess(filepathB);
    // loadProcess(filepathC);

    // switch (policy)
    // {
    // case FCFS:
    //     runFCFS();
    //     break;   
    // case RR:
    //     runRR();
    //     break;    
    // case MLFQ:
    //     runMLFQ();
    //     break;
    // default:
    //     break;
    // }

    


    


        

}

void initSemaphores(){

    sem_init(&userInputSemaphore, 0, 1);
    sem_init(&userOutputSemaphore, 0, 1);
    sem_init(&fileSemaphore, 0, 1);


}


void executeSingleLinePCB(ProcessControlBlock* processControlBlock){

    int memoryStart = processControlBlock->memory_start;
    int currentLine = memoryStart + processControlBlock->program_counter + CODE_SEGMENT_OFFSET;

    char* line = mainMemory.memoryArray[currentLine].data;
    char** tokens = lineParser(line);
    
    /*
        Tokens:
            assign
            semWait
            semSignal
            print
            printFromTo
    */

    if (strcmp(tokens[0], "assign") == 0) {
        char* target = tokens[1];
        char* value = tokens[2];

        if(strcmp(value, "readfile") == 0){

            /*
            Handle Case for 
                assign a readfile b
            */

            char* filepath = tokens[3];
            char* fileString = readFile(filepath);

            if(strcmp(target, "a") == 0){
                mainMemory.memoryArray[processControlBlock->memory_start + 6].data = fileString;
            }else if (strcmp(target, "b") == 0){
                mainMemory.memoryArray[processControlBlock->memory_start + 7].data = fileString;
            }else {
                mainMemory.memoryArray[processControlBlock->memory_start + 8].data = fileString;
            }

        }else if (strcmp(value, "input") == 0){

            /*
            Handle Case for 
                assign a input
            */
            char* userInputValue;
            printf("Please enter a value:\n");
            scanf("%s", userInputValue);

            if(strcmp(target, "a") == 0){
                mainMemory.memoryArray[processControlBlock->memory_start + 6].data = userInputValue;
            }else if (strcmp(target, "b") == 0){
                mainMemory.memoryArray[processControlBlock->memory_start + 7].data = userInputValue;
            }else {
                mainMemory.memoryArray[processControlBlock->memory_start + 8].data = userInputValue;
            }

            
        } else {
                        /*
            Handle Case for 
                assign a b
            */

            if(strcmp(target, "a") == 0){
                mainMemory.memoryArray[processControlBlock->memory_start + 6].data = value;
            }else if (strcmp(target, "b") == 0){
                mainMemory.memoryArray[processControlBlock->memory_start + 7].data = value;
            }else {
                mainMemory.memoryArray[processControlBlock->memory_start + 8].data = value;
            }

        }


        
    }
    else if (strcmp(tokens[0], "print") == 0) {
        char* var = tokens[1];
        // TODO: handle print
    }
    else if (strcmp(tokens[0], "printFromTo") == 0) {
        char* start = tokens[1];
        char* end = tokens[2];
        // TODO: handle printFromTo
    }
    else if (strcmp(tokens[0], "semWait") == 0) {
        char* resource = tokens[1];
        // TODO: handle semWait
    }
    else if (strcmp(tokens[0], "semSignal") == 0) {
        char* resource = tokens[1];
        // TODO: handle semSignal
    }
    else if (strcmp(tokens[0], "writeFile") == 0) {
        char* filenameVar = tokens[1];
        char* dataVar = tokens[2];
        // TODO: handle writeFile
    }
    else if (strcmp(tokens[0], "readFile") == 0) {
        char* filenameVar = tokens[1];
        // TODO: handle readFile
    }
    else {
        // Unknown instruction
        printf("Unknown instruction: %s\n", tokens[0]);
    }
    


    processControlBlock->program_counter += 1;

    

}

char** lineParser(char* line){

    char** tokenArray = (char**)malloc(sizeof(char*) * 16); //Assuming maximum of 8 tokens per line.
    char* token = strtok(line, " ");
    char lineCopy[strlen(line)];
    strcpy(lineCopy, line);


    int counter = 0;

    while(token != NULL){
        tokenArray[counter++] = strdup(token);
        token = strtok(NULL, " ");

    }
    tokenArray[counter] = NULL;

    return tokenArray;

}

void safe_sem_post(sem_t* __sem){

    int value;
    sem_getvalue(__sem, &value);

    if(value >= 1) return;

    sem_post(__sem);


}

void safe_sem_wait(sem_t* __sem){

    int value;
    sem_getvalue(__sem, &value);

    if(value <= 0) return;

    sem_wait(__sem);


}

void initMemory() {
    for (int i = 0; i < 60; i++) {
        // Allocate memory for name and data
        mainMemory.memoryArray[i].name = malloc(64 * sizeof(char)); // or any size you need
        mainMemory.memoryArray[i].data = malloc(64 * sizeof(char)); // or any size you need

        // Initialize name and data
        mainMemory.memoryArray[i].name[0] = '\0'; // Empty string initialization
        mainMemory.memoryArray[i].data[0] = '\0'; // Empty string initialization
    }
}


ProcessControlBlock loadProcess(char* filepath){

    ProcessControlBlock pcb;

    int indexOfFree = 0;
    while(strcmp(mainMemory.memoryArray[indexOfFree].name, "")){
        indexOfFree++;
    }

    pcb.id = processIdCounter;
    processIdCounter++;

    pcb.memory_start = indexOfFree;
    pcb.memory_end = indexOfFree + 20;

    pcb.priority = 1;

    pcb.program_counter = 0;

    pcb.state = READY;

    char buffer[64];

    mainMemory.memoryArray[pcb.memory_start + 0].name = "pid";
    sprintf(buffer, "%d", pcb.id);
    strcpy(mainMemory.memoryArray[pcb.memory_start + 0].data, buffer);

    mainMemory.memoryArray[pcb.memory_start + 1].name = "mem_start";
    sprintf(buffer, "%d", pcb.memory_start);
    strcpy(mainMemory.memoryArray[pcb.memory_start + 1].data, buffer);

    mainMemory.memoryArray[pcb.memory_start + 2].name = "mem_end";
    sprintf(buffer, "%d", pcb.memory_end);
    strcpy(mainMemory.memoryArray[pcb.memory_start + 2].data, buffer);

    mainMemory.memoryArray[pcb.memory_start + 3].name = "priority";
    sprintf(buffer, "%d", pcb.priority);
    strcpy(mainMemory.memoryArray[pcb.memory_start + 3].data, buffer);

    mainMemory.memoryArray[pcb.memory_start + 4].name = "program_counter";
    sprintf(buffer, "%d", pcb.program_counter);
    strcpy(mainMemory.memoryArray[pcb.memory_start + 4].data, buffer);

    mainMemory.memoryArray[pcb.memory_start + 5].name = "state";
    sprintf(buffer, "%s", processToString(pcb.state));
    strcpy(mainMemory.memoryArray[pcb.memory_start + 5].data, buffer);

    mainMemory.memoryArray[pcb.memory_start + 6].name = "a";
    mainMemory.memoryArray[pcb.memory_start + 7].name = "b";
    mainMemory.memoryArray[pcb.memory_start + 8].name = "c";

    FILE* code_file = fopen(filepath, "r");

    if(code_file == NULL){
        perror("Unable to open file");
    }
    
    
    int code_segment_line = 0;
    while(fgets(buffer, 64, code_file)){
        
        mainMemory.memoryArray[pcb.memory_start + CODE_SEGMENT_OFFSET + code_segment_line].name = "code";
        strcpy(mainMemory.memoryArray[pcb.memory_start + CODE_SEGMENT_OFFSET + code_segment_line].data, buffer);
        code_segment_line++;
    }

    return pcb;


}

char* processToString(ProcessState state){
    switch (state)
    {
    case READY:
        return "ready";
        break;

    case BLOCKED:
        return "blocked";
        break;

    case RUNNING:
        return "running";
        break;
    
    default:
        break;
    }
}


void displayMemory(Memory* mainMemory) {
    printf("Memory Contents:\n");

    for (int i = 0; i < 60; i++) {
        // Check if name and data are valid (not NULL)
        if (mainMemory->memoryArray[i].name != NULL && mainMemory->memoryArray[i].data != NULL) {
            printf("Index %d: { Key: '%s', Value: '%s' }\n", i, mainMemory->memoryArray[i].name, mainMemory->memoryArray[i].data);
        } else {
            printf("Index %d: { Key: NULL, Value: NULL }\n", i);
        }
    }
}