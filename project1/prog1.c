
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define N 500

int matA[N][N];
int matB[N][N];
int matC[N][N];

void multiply(int matA[N][N], int matB[N][N], int matC[N][N])
{
    int i, j, k;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            for (k = 0; k < N; k++)
            matC[i][j] += matA[i][k]*matB[k][j];
            usleep(10);
            
        }
    }
}
int main()
{   
    int i, j;
    srand(1);
    for ( i = 0; i < N; i++) 
    {
        for ( j = 0; j < N; j++) 
        {
            matA[i][j] = rand()%2001 ;
            matB[i][j] = rand()%2001 ;
        }
    }

    multiply(matA, matB, matC);
     


    return 0;
}

