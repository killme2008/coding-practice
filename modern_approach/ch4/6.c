#include<stdio.h>

int main(int argc, char *argv[])
{
  int i, j, k;

  i=j=1;
  k = ++i+j--;
  //2 0 3
  printf("%d %d %d\n", i, j, k);

  return 0;
}
