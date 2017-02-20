#include <stdio.h>

int main(int argc, char *argv[])
{
  /* %zu c99 only */
  /* long long c99 only */
  printf("sizeof(int)=%zu\n", sizeof(int));
  printf("sizeof(short)=%zu\n", sizeof(short));
  printf("sizeof(long)=%zu\n", sizeof(long));
  printf("sizeof(long long)=%zu\n", sizeof(long long));
  printf("sizeof(float)=%zu\n", sizeof(float));
  printf("sizeof(double)=%zu\n", sizeof(double));
  printf("sizeof(long double)=%zu\n", sizeof(long double));
  return 0;
}
