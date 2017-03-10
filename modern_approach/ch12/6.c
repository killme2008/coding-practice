#include <stdio.h>

int sum_array(const int *a, int n) {
  int* p = a;
  int sum = 0;
  while(p < a+n){
    sum += *p++;
  }
  return sum;
}

int main(int argc, char *argv[])
{
  int a[] = {1,2,3,4,5,6,7,8,9,10};
  int sum = sum_array(a, 10);
  printf("sum=%d", sum);
  return 0;
}
