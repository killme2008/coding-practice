#include <stdio.h>

int* find_largest(int a[],int n);

int main(int argc, char *argv[])
{
  int i;
  int a[10];

  for(i = 0; i < 10; i++)
    a[i] = (i % 2 == 0) ? i*2 : i ;

  printf("largest = %d", *find_largest(a, 10));
  return 0;
}

int* find_largest(int a[],int n) {
  int* ret = a;
  for(int i = 1; i < n; i++)
    if(a[i] > *ret)
      ret = a+i;
  return ret;
}
