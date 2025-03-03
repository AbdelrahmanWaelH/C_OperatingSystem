#include <stdio.h>
#include <pthread.h>

void inBetweenChars()
{ // Thread 1 start
    char start, end;
    printf("Enter the start character: ");
    scanf(" %c", &start);
    printf("Enter the end character: ");
    scanf(" %c", &end);

    /*if(start > end) swap(start, end)*/

    for (; start <= end; start++)
    {
        printf("%c\n", start);
    }
} // Thread 1 end

void sumAvgProduct()
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

    for (; start <= end; start++)
    {
        sum += start;
        product *= start;
    }

    average = (float)sum / delta;
    printf("\nThe sum is : %d", sum);
    printf("\nThe product is : %d", product);
    printf("\nThe average is : %f\n", average);

} // Thread 3 end

int main()
{
    inBetweenChars();
    sumAvgProduct();
    return 0;
}