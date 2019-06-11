#include <stdio.h>
#include <string.h>

int main(void)
{

    float A[16] = {7,85,14,3,
             0,1,2,20,
             7,4,3,18,
             26,75,94,47
             };

   float B[16] = {1,2,3,4,
             5,6,7,8,
             9,10,11,12,
             13,14,15,16
             };

    float result[16];

    int i;
    int j;
    int k;
    float sum;

    for (i=0; i<4; i++)
    {
        for (j=0; j<4; j++)
        {
            sum = 0;
            for(k = 0; k<4; k++)
                sum +=  A[(i<<2) + k] * B[(k<<2) + j];
            result[(i<<2) + j] = sum;
        }
    }
/*
    for(i = 0; i < 4; ++i)
    {
        for(j = 0; j < 4; ++j)
        {
                printf("%d ", result[(i<<2) + j]);
        }
        printf("\n");
    }
*/
    return 0;
}
