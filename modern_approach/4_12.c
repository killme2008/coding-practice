#include<stdio.h>

int main(int argc, char *argv[])
{
  int i,j;

  i=5;
  j=++i*3-2;
  printf("%d %d\n", i, j);

  i=5;
  j=3-2*i++;
  printf("%d %d\n", i, j);

  i=7;
  j=3*i--+2;
  printf("%d %d\n", i, j);

  i=7;
  j=3+--i*2;
  printf("%d %d\n", i, j);

  return 0;
}
