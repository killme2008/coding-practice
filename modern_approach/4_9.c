#include<stdio.h>

int main(){
  int i, j, k;

  i=j=k=1;
  i += j += k;
  printf("%d %d %d\n", i, j, k);

  i=1;j=2;k=3;
  i -= j -= k;
  printf("%d %d %d\n", i, j, k);

  i=2;j=1;k=0;
  i *= j *= k;
  printf("%d %d %d\n", i, j, k);

  return 0;
}
