#include <stdio.h>
#include <string.h>

float A[10] = {89.53, 40.6, 323.63, 4267.65, 266.59, 25.451, 907.543, 602.543, 568.641, 462.2};

int partition(int low, int hi)
{
    float pivot = A[hi];
    int i = low-1,j;
    float  temp;
    for(j = low; j<hi; j++)
    {
        if(A[j] < pivot)
        {
            i = i+1;
            temp = A[i];
            A[i] = A[j];
            A[j] = temp;
        }
    }
    if(A[hi] < A[i+1])
    {
        temp = A[i+1];
        A[i+1] = A[hi];
        A[hi] = temp;
    }
    return i+1;
}

void qsort(int low, int hi)
{
    if(low < hi)
    {
        int p = partition(low, hi);
        qsort(low,p-1);
        qsort(p+1,hi);
    }
}

int main()
{
    int i;
    qsort(0,10-1);
    for(i = 0; i < 10; ++i)
    {
      printf("%f\n",   A[i]);
    }
    return 0;
}
