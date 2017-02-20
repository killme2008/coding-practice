#include <stdio.h>

int main(int argc, char *argv[])
{
  char a[8][8];
  int i, j;

  for(i=0;i<8;i++)
    for(j=0;j<8;j++) {
      if((i+j) % 2 ==0)
        a[i][j] = 'B';
      else
        a[i][j] = 'R';
    }

  for(i=0;i<8;i++) {
    for(j=0;j<8;j++) {
      printf("%c ", a[i][j]);
    }
    printf("\n");
  }

  return 0;
}
