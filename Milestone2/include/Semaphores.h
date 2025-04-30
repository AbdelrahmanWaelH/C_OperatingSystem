#include <semaphore.h>
#include <stdlib.h>

extern sem_t userInputSemaphore;
extern sem_t userOutputSemaphore;
extern sem_t fileSemaphore;

void initSemaphores();
