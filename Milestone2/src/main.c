#include "../include/FileReader.h"
#include "../include/Queue.h"
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODE_SEGMENT_OFFSET 9
#define MLFQ_LEVELS 4

typedef enum { SCHED_FCFS, SCHED_RR, SCHED_MLFQ } SchedulerType;

typedef struct Scheduler {
	SchedulerType type;
	Queue* input_queues[MLFQ_LEVELS];
	Queue* output_queue;
	int mlfq_quantum[MLFQ_LEVELS];
	int rr_quantum;
} Scheduler;

typedef enum { READY, RUNNING, BLOCKED } ProcessState;

typedef enum {
	USER_INPUT,
	USER_OUTPUT,
	FILE_RW,
	NIL,
} Resource;

typedef struct {
	int id;
	ProcessState state;
	int priority;
	int memory_start;
	int memory_end;
	int program_counter;

	// virtual values
	Resource blockedResource;
	int timeslice_used;

} ProcessControlBlock; // 6 words

typedef struct {

	char name[256];
	char data[256];

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

/*

	1. Load each process into memory (create PCB, declare variables, load
   program)
	2. Begin scheduling using specified algorithm.
	3.

*/

extern sem_t userInputSemaphore;
extern sem_t userOutputSemaphore;
extern sem_t fileSemaphore;

void removeNewline(char* buffer)
{
	// Find the newline character (if any) and replace it with '\0'
	size_t length = strlen(buffer);
	if (length > 0 && buffer[length - 1] == '\n') {
		buffer[length - 1] = '\0';
	}
}

// void initSemaphores(){

//     if (sem_init(&userInputSemaphore, 0, 1) == -1) {
//         perror("sem_init userInput");
//         exit(EXIT_FAILURE);
//     }
//     sem_init(&userOutputSemaphore, 0, 1);
//     sem_init(&fileSemaphore, 0, 1);

// }

Scheduler* scheduler_new(SchedulerType type, ...);
void scheduler_add_task(Scheduler* sched, ProcessControlBlock* process);
void scheduler_step(Scheduler* sched);
void scheduler_free(Scheduler* sched);

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

int processIdCounter = 0;

Memory mainMemory;
Queue blockedQueue;

char* filepathA = "../Program_1.txt";
char* filepathB = "../Program_2.txt";
char* filepathC = "../Program_3.txt";

int main()
{

	// Prompt user for scheduling algo.
	initMemory();
	initSemaphores();
	ProcessControlBlock pcbA = loadProcess(filepathA);
	ProcessControlBlock pcbB = loadProcess(filepathB);
	ProcessControlBlock pcbC = loadProcess(filepathC);

	Scheduler* sched = scheduler_new(SCHED_FCFS); // Change to SCHED_FCFS or SCHED_RR or SCHED_MLFQ

	scheduler_add_task(sched, &pcbA);
	scheduler_add_task(sched, &pcbB);
	scheduler_add_task(sched, &pcbC);

	int i = 0;
	while (!queue_is_empty(sched->input_queues[0]) || !queue_is_empty(sched->input_queues[1])
		|| !queue_is_empty(sched->input_queues[2]) || !queue_is_empty(sched->input_queues[3])) {
		// ProcessControlBlock* process
		// 	= (ProcessControlBlock*)queue_peek_head(sched->input_queues[0]);
		printf("=== Cycle %d ===\n", i++);
		scheduler_step(sched);
	}

	// while(pcbA.program_counter + CODE_SEGMENT_OFFSET + pcbA.memory_start <=
	// pcbA.memory_end){
	//     executeSingleLinePCB(&pcbA);
	// }
	//
	// printf("\n==Finished A==\n");
	//
	// while(pcbB.program_counter + CODE_SEGMENT_OFFSET + pcbB.memory_start <=
	// pcbB.memory_end){
	//     executeSingleLinePCB(&pcbB);
	// }
	//
	// printf("\n==Finished B==\n");
	//
	// // int val1;
	// // int val2;
	// // int val3;
	//
	// // printf("Getting sem values...\n");
	//
	// // sem_getvalue(&userInputSemaphore, &val1);
	// // sem_getvalue(&userOutputSemaphore, &val2);
	// // sem_getvalue(&fileSemaphore, &val3);
	//
	// // printf("Semaphore values: %d, %d, %d\n", val1, val2, val3);
	//
	// // printf("Done\n");
	//
	//
	// while(pcbC.program_counter + CODE_SEGMENT_OFFSET + pcbC.memory_start <=
	// pcbC.memory_end){
	//     executeSingleLinePCB(&pcbC);
	// }
	//
	// printf("\n==Finished C==\n");
	// //displayMemory(&mainMemory);
}

void executeSingleLinePCB(ProcessControlBlock* processControlBlock)
{

	int memoryStart = processControlBlock->memory_start;
	int currentLine = memoryStart + processControlBlock->program_counter + CODE_SEGMENT_OFFSET;

	// printf("%d\n", processControlBlock->id);

	char* line = mainMemory.memoryArray[currentLine].data;
	char** tokens = lineParser(line);

	// printf("Currently at PC: %d\n", processControlBlock->program_counter);

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

		value[strcspn(value, "\r\n")] = '\0';

		if (strcmp(value, "readFile") == 0) {
			/*
			Handle Case for
				assign a readfile b
			*/

			char* filepathVar = tokens[3];
			// removeNewline(filepathVar);
			filepathVar[strcspn(filepathVar, "\r\n")] = '\0';

			char filepath[256];

			if (strcmp(filepathVar, "a") == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 6].data =
				// fileString;
				strcpy(
					filepath, mainMemory.memoryArray[processControlBlock->memory_start + 6].data);
			} else if (strcmp(filepathVar, "b") == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 7].data =
				// fileString;
				strcpy(
					filepath, mainMemory.memoryArray[processControlBlock->memory_start + 7].data);
			} else {
				// mainMemory.memoryArray[processControlBlock->memory_start + 8].data =
				// fileString;
				strcpy(
					filepath, mainMemory.memoryArray[processControlBlock->memory_start + 8].data);
			}

			char* fileString = readFile(filepath);

			// printf("Read File String: %s", fileString);

			// printf("Found file path\n");

			if (strcmp(target, "a") == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 6].data =
				// fileString;
				strcpy(
					mainMemory.memoryArray[processControlBlock->memory_start + 6].data, fileString);
			} else if (strcmp(target, "b") == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 7].data =
				// fileString;
				strcpy(
					mainMemory.memoryArray[processControlBlock->memory_start + 7].data, fileString);
			} else {
				// mainMemory.memoryArray[processControlBlock->memory_start + 8].data =
				// fileString;
				strcpy(
					mainMemory.memoryArray[processControlBlock->memory_start + 8].data, fileString);
			}

		} else if (strcmp(value, "input") == 0) {

			/*
			Handle Case for
				assign a input
			*/

			char userInputValue[256] = { 0 };
			printf("Please enter a value (process = %d):\n", processControlBlock->id);
			char buffer[256] = { 0 };
			fgets(buffer, 256, stdin);
			buffer[strcspn(buffer, "\r\n")] = '\0';
			strcpy(userInputValue, buffer);

			if (strcmp(target, "a") == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 6].data =
				// userInputValue;
				strcpy(mainMemory.memoryArray[processControlBlock->memory_start + 6].data,
					userInputValue);

			} else if (strcmp(target, "b") == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 7].data =
				// userInputValue;
				strcpy(mainMemory.memoryArray[processControlBlock->memory_start + 7].data,
					userInputValue);
			} else {
				// mainMemory.memoryArray[processControlBlock->memory_start + 8].data =
				// userInputValue;
				strcpy(mainMemory.memoryArray[processControlBlock->memory_start + 8].data,
					userInputValue);
			}

		} else {
			/*
	  Handle Case for
	  assign a b
	  */

			int indexOfTarget = 0;
			int indexOfValue = 0;
			for (int i = 6; i <= 8; i++) {
				if (strcmp(
						mainMemory.memoryArray[processControlBlock->memory_start + i].name, target)
					== 0) {
					indexOfTarget = i;
				}
				if (strcmp(
						mainMemory.memoryArray[processControlBlock->memory_start + i].name, value)
					== 0) {
					indexOfValue = i;
				}
			}

			// mainMemory.memoryArray[processControlBlock->memory_start +
			// indexOfTarget].data =
			// mainMemory.memoryArray[processControlBlock->memory_start +
			// indexOfValue].data;
			strcpy(mainMemory.memoryArray[processControlBlock->memory_start + indexOfTarget].data,
				mainMemory.memoryArray[processControlBlock->memory_start + indexOfValue].data);
		}

	} else if (strcmp(tokens[0], "print") == 0) {
		char* value = tokens[1];
		char data[256];

		if (strcmp(value, "a") == 0) {
			// mainMemory.memoryArray[processControlBlock->memory_start + 6].data =
			// userInputValue;
			strcpy(data, mainMemory.memoryArray[processControlBlock->memory_start + 6].data);

		} else if (strcmp(value, "b\r\n") == 0) {
			// mainMemory.memoryArray[processControlBlock->memory_start + 7].data =
			// userInputValue;
			strcpy(data, mainMemory.memoryArray[processControlBlock->memory_start + 7].data);
		} else {
			// mainMemory.memoryArray[processControlBlock->memory_start + 8].data =
			// userInputValue;
			strcpy(data, mainMemory.memoryArray[processControlBlock->memory_start + 8].data);
		}

		printf("%s\n", data);
	} else if (strcmp(tokens[0], "printFromTo") == 0) {
		char* start = tokens[1];
		char* end = tokens[2];

		end[strcspn(end, "\r\n")] = '\0';

		int indexOfValue = 0;
		char* dataStart;
		char* dataEnd;

		// displayMemory(&mainMemory);

		for (int i = 6; i <= 8; i++) {
			if (strcmp(mainMemory.memoryArray[processControlBlock->memory_start + i].name, start)
				== 0) {
				dataStart = mainMemory.memoryArray[processControlBlock->memory_start + i].data;
			}
		}
		for (int i = 6; i <= 8; i++) {

			if (strcmp(mainMemory.memoryArray[processControlBlock->memory_start + i].name, end)
				== 0) {
				dataEnd = mainMemory.memoryArray[processControlBlock->memory_start + i].data;
			}
		}

		int startInt = atoi(dataStart);
		int endInt = atoi(dataEnd);

		for (int i = startInt; i <= endInt; i++) {
			printf("%d ", i);
		}
		printf("\n");

	} else if (strcmp(tokens[0], "semWait") == 0) {
		char* resource = tokens[1];
		int semValue;
		int* pid = malloc(sizeof(int));
		*pid = processControlBlock->id;
		if (strcmp(resource, "file") == 0) {

			sem_getvalue(&fileSemaphore, &semValue);
			if (semValue == 0) {
				processControlBlock->state = BLOCKED;
				processControlBlock->blockedResource = FILE_RW;
				return;
			} else {
				safe_sem_wait(&fileSemaphore);
			}

		} else if (strcmp(resource, "userInput") == 0) {
			sem_getvalue(&userInputSemaphore, &semValue);
			if (semValue == 0) {
				processControlBlock->state = BLOCKED;
				processControlBlock->blockedResource = USER_INPUT;
				return;
			} else {
				safe_sem_wait(&userInputSemaphore);
			}
		} else if (strcmp(resource, "userOutput") == 0) {
			sem_getvalue(&userOutputSemaphore, &semValue);
			if (semValue == 0) {
				processControlBlock->state = BLOCKED;
				processControlBlock->blockedResource = USER_OUTPUT;
				return;
			} else {
				safe_sem_wait(&userOutputSemaphore);
			}
		}

	} else if (strcmp(tokens[0], "semSignal") == 0) {
		char* resource = tokens[1];
		if (strcmp(resource, "file") == 0) {
			safe_sem_post(&fileSemaphore);
		} else if (strcmp(resource, "userInput")) {
			safe_sem_post(&userInputSemaphore);
		} else if (strcmp(resource, "userOutput")) {
			safe_sem_post(&userOutputSemaphore);
		}

	} else if (strcmp(tokens[0], "writeFile") == 0) {
		char* filenameVar = tokens[1];
		char* dataVar = tokens[2];
		dataVar[strcspn(dataVar, "\r\n")] = '\0';

		char* dataToWrite;
		char* filename;

		for (int i = 6; i <= 8; i++) {
			if (strcmp(mainMemory.memoryArray[processControlBlock->memory_start + i].name, dataVar)
				== 0) {
				dataToWrite = mainMemory.memoryArray[processControlBlock->memory_start + i].data;
			}
			if (strcmp(
					mainMemory.memoryArray[processControlBlock->memory_start + i].name, filenameVar)
				== 0) {
				filename = mainMemory.memoryArray[processControlBlock->memory_start + i].data;
			}
		}

		FILE* newFile = fopen(filename, "w");
		fprintf(newFile, "%s", dataToWrite);
		fflush(newFile);

	}

	else {
		// Unknown instruction
		printf("Unknown instruction: %s\n", tokens[0]);
	}

	processControlBlock->program_counter += 1;
}

char** lineParser(char* line)
{

	char** tokenArray
		= (char**)malloc(sizeof(char*) * 16); // Assuming maximum of 8 tokens per line.

	char lineCopy[strlen(line) + 10];
	strcpy(lineCopy, line);
	char* token = strtok(lineCopy, " ");

	int counter = 0;

	while (token != NULL) {
		tokenArray[counter++] = strdup(token);
		token = strtok(NULL, " ");
	}
	tokenArray[counter] = NULL;

	return tokenArray;
}

void safe_sem_post(sem_t* __sem)
{

	int value;
	sem_getvalue(__sem, &value);

	if (value >= 1)
		return;

	sem_post(__sem);
}

void safe_sem_wait(sem_t* __sem)
{

	int value;
	sem_getvalue(__sem, &value);

	if (value <= 0)
		return;

	sem_wait(__sem);
}

void initMemory()
{
	for (int i = 0; i < 60; i++) {

		// mainMemory.memoryArray[i].name = malloc(64 * sizeof(char)); // or any
		// size you need mainMemory.memoryArray[i].data = malloc(64 * sizeof(char));
		// // or any size you need
	}
}

ProcessControlBlock loadProcess(char* filepath)
{

	ProcessControlBlock pcb;

	int indexOfFree = 0;
	while (strcmp(mainMemory.memoryArray[indexOfFree].name, "")) {
		indexOfFree++;
	}

	if (indexOfFree > 40) {
		perror("Not enough memory to store process in memory");
		return pcb;
	}

	pcb.id = processIdCounter;
	processIdCounter++;

	pcb.memory_start = indexOfFree;

	pcb.priority = 1;

	pcb.program_counter = 0;

	pcb.state = READY;

	char* buffer = malloc(256);

	// mainMemory.memoryArray[pcb.memory_start + 0].name = "pid";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 0].name, "pid");
	sprintf(buffer, "%d", pcb.id);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 0].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 1].name = "mem_start";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 1].name, "mem_start");
	sprintf(buffer, "%d", pcb.memory_start);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 1].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 2].name = "mem_end";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 2].name, "mem_end");
	sprintf(buffer, "%d", pcb.memory_end);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 2].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 3].name = "priority";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 3].name, "priority");
	sprintf(buffer, "%d", pcb.priority);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 3].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 4].name = "program_counter";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 4].name, "program_counter");
	sprintf(buffer, "%d", pcb.program_counter);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 4].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 5].name = "state";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 5].name, "state");
	sprintf(buffer, "%s", processToString(pcb.state));
	strcpy(mainMemory.memoryArray[pcb.memory_start + 5].data, buffer);

	strcpy(mainMemory.memoryArray[pcb.memory_start + 6].name, "a");
	strcpy(mainMemory.memoryArray[pcb.memory_start + 7].name, "b");
	strcpy(mainMemory.memoryArray[pcb.memory_start + 8].name, "c");
	// mainMemory.memoryArray[pcb.memory_start + 6].name = "a";
	// mainMemory.memoryArray[pcb.memory_start + 7].name = "b";
	// mainMemory.memoryArray[pcb.memory_start + 8].name = "c";

	FILE* code_file = fopen(filepath, "r");

	if (code_file == NULL) {
		perror("Unable to open file");
	}

	int code_segment_line = 0;

	while (fgets(buffer, 256, code_file)) {

		strcpy(
			mainMemory.memoryArray[pcb.memory_start + CODE_SEGMENT_OFFSET + code_segment_line].name,
			"code");
		// memcpy(mainMemory.memoryArray[pcb.memory_start + CODE_SEGMENT_OFFSET +
		// code_segment_line].data, buffer, 256); printf("Buffer: %c\n", buffer[7]);

		for (int i = 0; i < 256; i++) {
			mainMemory.memoryArray[pcb.memory_start + CODE_SEGMENT_OFFSET + code_segment_line]
				.data[i]
				= buffer[i];
		}

		code_segment_line++;
	}
	code_segment_line--;
	pcb.memory_end = pcb.memory_start + CODE_SEGMENT_OFFSET + code_segment_line;
	printf("PID: %d, Start: %d, Lines of Code: %d, Memory End: %d\n", pcb.id, pcb.memory_start,
		code_segment_line, pcb.memory_end);

	strcpy(mainMemory.memoryArray[pcb.memory_start + 2].name, "mem_end");
	sprintf(buffer, "%d", pcb.memory_end);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 2].data, buffer);

	return pcb;
}

char* processToString(ProcessState state)
{
	switch (state) {
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

void displayMemory(Memory* mainMemory)
{
	printf("Memory Contents:\n");

	for (int i = 0; i < 60; i++) {
		// Check if name and data are valid (not NULL)
		if (mainMemory->memoryArray[i].name != NULL && mainMemory->memoryArray[i].data != NULL) {
			printf("Index %d: { Key: '%s', Value: '%s' }\n", i, mainMemory->memoryArray[i].name,
				mainMemory->memoryArray[i].data);
		} else {
			printf("Index %d: { Key: NULL, Value: NULL }\n", i);
		}
	}
}

// ================ Schedule Stuff ================

Scheduler* scheduler_new(SchedulerType type, ...)
{
	Scheduler* s = (Scheduler*)malloc(sizeof(Scheduler));
	s->type = type;
	va_list args;
	va_start(args, type);
	if (type == SCHED_RR)
		s->rr_quantum = va_arg(args, int);
	else
		s->rr_quantum = 0;
	va_end(args);

	for (int i = 0; i < MLFQ_LEVELS; i++)
		s->input_queues[i] = queue_new();
	s->output_queue = queue_new();
	int base = 1;
	for (int i = 0; i < MLFQ_LEVELS; i++)
		s->mlfq_quantum[i] = base << i; // 1, 2, 4, 8
	return s;
}

void scheduler_add_task(Scheduler* sched, ProcessControlBlock* process)
{
	queue_push_tail(sched->input_queues[0], process);
}

void scheduler_step(Scheduler* sched)
{
	int semValue = 0;

	if (!queue_is_empty(&blockedQueue) && sched->type != SCHED_FCFS) {
		Queue* temp = queue_new();

		while (!queue_is_empty(&blockedQueue)) {
			ProcessControlBlock* process
				= (ProcessControlBlock*)queue_pop_head(sched->input_queues[0]);

			switch (process->blockedResource) {
			case FILE_RW: {
				sem_getvalue(&fileSemaphore, &semValue);
				break;
			}
			case USER_INPUT: {
				sem_getvalue(&userInputSemaphore, &semValue);
				break;
			}
			case USER_OUTPUT: {
				sem_getvalue(&userOutputSemaphore, &semValue);
				break;
			}
			default:
				break;
			}

			if (semValue != 0) {
				process->blockedResource = NIL;
				process->state = READY;
				process->timeslice_used = 0;
				queue_push_tail(sched->input_queues[0], process);
			} else {
				queue_push_tail(temp, process);
			}
		}
		blockedQueue = *temp;
	}

	switch (sched->type) {
	case SCHED_FCFS: {
		if (!queue_is_empty(sched->input_queues[0])) {
			ProcessControlBlock* process
				= (ProcessControlBlock*)queue_peek_head(sched->input_queues[0]);
			executeSingleLinePCB(process);

			if (process->memory_start + process->program_counter + CODE_SEGMENT_OFFSET
				>= process->memory_end) {
				queue_pop_head(sched->input_queues[0]);
				queue_push_tail(sched->output_queue, process);
			}
		}
		break;
	}

	case SCHED_RR: {
		if (!queue_is_empty(sched->input_queues[0])) {
			ProcessControlBlock* process
				= (ProcessControlBlock*)queue_peek_head(sched->input_queues[0]);
			executeSingleLinePCB(process);
			process->timeslice_used++;

			if (process->state == BLOCKED) {
				queue_pop_head(sched->input_queues[0]);
				queue_push_tail(&blockedQueue, process);
				return;
			}

			if (process->memory_start + process->program_counter + CODE_SEGMENT_OFFSET
				>= process->memory_end) {
				queue_pop_head(sched->input_queues[0]);
				queue_push_tail(sched->output_queue, process);
			} else if (process->timeslice_used >= sched->rr_quantum) {
				process->timeslice_used = 0;
				queue_pop_head(sched->input_queues[0]);
				queue_push_tail(sched->input_queues[0], process);
			}
		}
		break;
	}

	case SCHED_MLFQ: {
		for (int level = 0; level < MLFQ_LEVELS; level++) {
			if (!queue_is_empty(sched->input_queues[level])) {
				ProcessControlBlock* process
					= (ProcessControlBlock*)queue_peek_head(sched->input_queues[level]);
				executeSingleLinePCB(process);
				process->timeslice_used++;

				if (process->state == BLOCKED) {
					queue_pop_head(sched->input_queues[0]);
					queue_push_tail(&blockedQueue, process);
					return;
				}

				if (process->memory_start + process->program_counter + CODE_SEGMENT_OFFSET
					>= process->memory_end) {
					process = (ProcessControlBlock*)queue_pop_head(sched->input_queues[level]);
					process->timeslice_used = 0;
					queue_push_tail(sched->output_queue, process);

				} else if (process->timeslice_used >= sched->mlfq_quantum[level]) {
					process = (ProcessControlBlock*)queue_pop_head(sched->input_queues[level]);
					process->timeslice_used = 0;
					int next_level = (level < MLFQ_LEVELS - 1) ? level + 1 : level;
					queue_push_tail(sched->input_queues[next_level], process);
				}
				break;
			}
		}
		break;
	}
	}
}
