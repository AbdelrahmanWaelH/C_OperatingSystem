#include "../include/Semaphores.h"

sem_t userInputSemaphore;
sem_t userOutputSemaphore;
sem_t fileSemaphore;
 
void initSemaphores(){

    sem_init(&userInputSemaphore, 0, 1);
    sem_init(&userOutputSemaphore, 0, 1);
    sem_init(&fileSemaphore, 0, 1);

}
