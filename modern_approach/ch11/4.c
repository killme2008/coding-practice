#include <stdio.h>

void swap(int* p, int* q);

int main(int argc, char *argv[])
{
  int i = 9, j=-9;
  swap(&i, &j);
  printf("i=%d, j=%d\n", i, j);
  return 0;
}

void swap(int* p, int* q) {
  int temp = *p;
  *p = *q;
  *q = temp;
}
