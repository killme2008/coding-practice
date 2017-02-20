#include <stdio.h>
#define N 500

int main(int argc, char *argv[])
{
  char ch;
  char a[N], b[N];
  int i, j, k, f, n;
  i = j = 0;

  printf("Enter a sentence: ");

  ch = getchar();
  while(ch != '?' && ch != '.' && ch !='!'){
    a[n++] = ch;
    ch = getchar();
  }
  a[n] = ch;

  /**reverse the sentence.*/
  for(i = n-1; i >= 0; i--){
    b[j++] = a[i];
  }
  b[n] = a[n];

  i = j = 0;
  for(j=0;j<n+1;j++){
    if(b[j] != ' ' && j != n)
      continue;
    /* reverse every word*/
    for(k=i,f=j-1;k<f;k++,f--){
      ch = b[k];
      b[k] = b[f];
      b[f] = ch;
    }
    i = j + 1;
  }

  printf("Reversal of sentence: ");

  for(i=0;i<n+1;i++)
    printf("%c",b[i]);
  return 0;
}
