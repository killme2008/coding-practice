#include<stdio.h>

int main(int argc, char *argv[])
{
  int i = 1;
  while(i <= 128) {
    printf("%d\n",i);
    i *= 2;
  }
  return 0;
}
