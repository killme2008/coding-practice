#include <stdio.h>

#define ERROR(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

int main(int argc, char *argv[])
{
  ERROR("Range error: index= %d\n", 100);
  int i, j;
  i = 1;
  j = 2;
  ERROR("ERROR, i= %d, j = %d\n", i, j);
  return 0;
}
