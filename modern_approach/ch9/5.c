#include <stdio.h>

int digits(int);

int main(int argc, char *argv[])
{
  for(int i = 1; i < 10000; i*=5)
    printf("digits(%d) = %d\n", i, digits(i));
  return 0;
}

int digits(int n) {
  int ret = 0;

  while(n>0){
    ret++;
    n /= 10;
  }
  return ret;
}
