#include<stdio.h>

int main(){
  int i, j, k;

  i=1;
  printf("%d\n", i++ -1);
  printf("%d\n", i);

  i=10;j=5;
  printf("%d\n", i++ - ++j);
  printf("%d %d\n", i, j);

  i=7;j=8;
  printf("%d\n", i++ - --j);
  printf("%d %d\n", i, j);

  i=3;j=4;k=5;
  printf("%d\n", i++ - j++ + --k);
  printf("%d %d %d\n", i, j, k);

  return 0;
}
