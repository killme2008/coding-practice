#include <stdio.h>
#include "quicksort.h"

#define N 10

int main(int argc, char *argv[])
{
  int a[N];
  int i, low, high;
  printf("Enter 10 numbers to sort: ");
  for(i = 0; i < N; i++)
    scanf("%d", &a[i]);
  quick_sort(a, 0, N-1);
  printf("Sorted numbers:");
  for(i = 0; i < N; i++)
    printf(" %d", a[i]);
  return 0;
}
