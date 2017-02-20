#include <stdio.h>

int main(int argc, char *argv[])
{
  int i;
  //for(i=10; i>=1;i/=2)
  //    printf("%d" ,i);
  i=10;
  while (i>=1) {
    printf("%d" ,i);
    i/=2;
  }
  return 0;
}
