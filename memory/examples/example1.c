#include "../memorytracker.h"
#include <stdio.h>

int main(){
    int *ptr = malloc(sizeof(int) * 4);
    int *arrPtr[100];
    int i;
    ptr[0] = '0';
    ptr[1] = '1';
    ptr[2] = '2';
    ptr[3] = '3';

    printf("%d %d %d %d\n", ptr[0], ptr[1], ptr[2], ptr[3]);

    for(i = 0; i < 100; i++)
    {
        arrPtr[i] = malloc(10 * sizeof(int));
    }

    free(ptr);

    for(i = 0; i < 50; i++)
    {
        free(arrPtr[i]);
    }

    f_trackListAllocations();

    for(; i < 100; i++)
    {
        free(arrPtr[i]);
    }

    return 0;


}