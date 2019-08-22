#include <stdio.h>
#include <string.h>

int main(void)
{

    float A[16] = {7.5,8.5,51.54,3.41,
             0.1,1,2,20.4,
             7,4,3,1.148,
             26,75.4,94.74,47.44144727
             };

    float B[16] = {1,2,3,4,
             0.5,6.17,7,8.11,
             9,10,11.17,12.741,
             13,14,15.171,16.17
             };

    float result[16];

    int i=0;
    int j;
    int k;
    float sum;

    for(int z = 0; z < 2000; z++)
    for (i=0; i<4; i++)
    {
        for (j=0; j<4; j++)
        {
            sum = 0;
            for(k = 0; k<4; k++)
                sum += A[(i<<2) + k] * B[(k<<2) + j];
            result[(i<<2) + j] = sum;
        }
    }

    for(i = 0; i < 4; i++)
    {
	    for(j = 0; j < 4; j++)
	    {
		    printf("%f", result[(i<<2) +j]);
	    }
    } 
    return 0;
}
