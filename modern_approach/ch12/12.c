#include <stdio.h>

void find_two_largest(int* a, int n, int* largest, int* second_largest) {
  int* p;
  *largest = *second_largest = *a;
  for(p = a + 1; p < a + n; p++)
    if(*p > *largest) {
      *second_largest = *largest;
      *largest = *p;
    }
}

int main(int argc, char *argv[])
{
  int a[] = {1,2,3,4,5,6,7,8,9,10};

  int largest, second_largest;
  find_two_largest(a, 10, &largest, &second_largest);
  printf("largest=%d, second_largest=%d", largest, second_largest);
  return 0;
}
