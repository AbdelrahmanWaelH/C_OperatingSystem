#include <stdio.h>
#include <pthread.h>

void* inBetweenChars(void* arg)
{ // Thread 1 start
    char start, end;
    printf("Enter the start character: ");
    scanf(" %c", &start);
    //remove the \n at the end of the scanf
    scanf("%c");

    printf("Enter the end character: ");
    scanf(" %c", &end);
    //remove the \n at the end of the scanf
    scanf("%c");


    /*if(start > end) swap(start, end)*/

    while(start <= end){
        printf("%c\n", start);
        start++;
    }
} // Thread 1 end

void* sumAvgProduct(void* arg)
{ // Thread 3 start
    int start, end;
    int sum = 0;
    int product = 1;
    float average = 0;

    printf("Enter the start integer: ");
    scanf(" %d", &start);
    printf("Enter the end integer: ");
    scanf(" %d", &end);

    /*if(start > end) swap(start, end)*/

    int delta = end - start + 1;

    while(start <= end){
        sum += start;
        product *= start;
        start++;
    }

    average = (float)sum / delta;
    printf("\nThe sum is : %d", sum);
    printf("\nThe product is : %d", product);
    printf("\nThe average is : %f\n", average);

} // Thread 3 end

void* functionPrint(void* arg){
    unsigned long threadId = pthread_self();
    printf("In thread with id : %lu , First Print\n",threadId);
    printf("In thread with id : %lu , Second Print\n",threadId);
    printf("In thread with id : %lu , Third Print\n",threadId);
}

int main()
{
    // declare the 3 required threads
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    // create the first thread and make it run  inBetweenChars
    pthread_create(&thread1, NULL, inBetweenChars, NULL);

    // create the second thread and make it run functionPrint
    pthread_create(&thread2, NULL, functionPrint, NULL);

    // create the second thread and make it run sumAvgProduct
    pthread_create(&thread3, NULL, sumAvgProduct, NULL);

    // wait for each of the threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    return 0;
}