#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define MAX 500
#define MAX_THREAD 25

int  matA[MAX][MAX];
int  matB[MAX][MAX];
int  matC[MAX][MAX];
int  step_i = 0;

void *multiply(void*arg)
{
    int posi = (intptr_t)arg;
    

    int i, j, k;
    for (i = posi * MAX / 25; i < (posi + 1) * MAX / 25; i++)
    {
        for (j = 0; j < MAX; j++)
        {

            for (k = 0; k < MAX; k++)
                matC[i][j] += matA[i][k]*matB[k][j];
            usleep(10);
        }
    }
    return NULL;
}


int main()
{

    int i, j;
    
    srand(1);
      for ( i = 0; i < MAX; i++)
      {
        for ( j = 0; j < MAX; j++)
        {
            matA[i][j] = rand() % 2001 ;
            matB[i][j] = rand() % 2001 ;
        }
    }
    
    pthread_t threads[MAX_THREAD];
    
    for ( i = 0; i < MAX_THREAD; i++)
        pthread_create(&threads[i], NULL, multiply, (void*)(intptr_t) i);
    for ( i = 0; i < MAX_THREAD; i++)
        pthread_join(threads[i], NULL);

    
    return 0;
}
