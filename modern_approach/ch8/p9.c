#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define DIRECTIONS 4
#define STEPS 26
#define N 10

int main(int argc, char *argv[])
{
  char a[N][N];
  int dirs[DIRECTIONS];
  int i, j, n, d;
  bool step;

  for(i = 0; i < N; i++)
    for(j = 0; j < N; j++)
      a[i][j] = '.';


  srand((unsigned)time(NULL));


  a[0][0] = 'A';
  i = j = n = d = 0;
  while(n<STEPS-1) {
    d = rand() % DIRECTIONS;
    step = false;
    switch (d) {
    case 0:
      if(i-1>=0 && a[i-1][j] == '.'){
        i--;
        step = true;
      }
      break;
    case 1:
      if(j+1<N && a[i][j+1] == '.'){
        j++;
        step = true;
      }
      break;
    case 2:
      if(i+1<N && a[i+1][j] == '.'){
        i++;
        step = true;
      }
      break;
    case 3:
      if(j-1>=0 && a[i][j-1] == '.'){
        j--;
        step = true;
      }
      break;
    }
    dirs[d] = 1;
    if(step){
      n++;
      a[i][j] = 'A' + n;
      for(int i=0;i<DIRECTIONS;i++)
        dirs[i] = 0;
    } else {
      bool t=true;
      for(int i=0;i<DIRECTIONS;i++)
        t = t && (dirs[i] == 1);
      if(t)
        break;
    }
  }

  for(i = 0; i < N; i++) {
    for(j = 0; j < N; j++)
      printf("%c ", a[i][j]);
    printf("\n");
  }

  return 0;
}
