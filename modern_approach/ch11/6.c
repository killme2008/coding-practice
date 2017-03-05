#include <stdio.h>

void find_two_largest(int a[],int n, int* largest, int* second_largest);

int main(int argc, char *argv[])
{
  int i;
  int a[10];

  for(i = 0; i < 10; i++)
    a[i] = (i % 2 == 0) ? i*2 : i ;

  int largest, second_largest;

  find_two_largest(a, 10, &largest, &second_largest);

  printf("largest=%d, second_largest=%d", largest, second_largest);
  return 0;
}

void find_two_largest(int a[],int n, int* largest, int* second_largest) {
  *largest = *second_largest = a[0];
  for(int i = 1; i < n; i++) {
    if(a[i] > *largest) {
      *second_largest = *largest;
      *largest = a[i];
    }
  }
}
