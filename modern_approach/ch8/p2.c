#include <stdio.h>

int main(int argc, char *argv[])
{
  int n[10] = {0};
  int i;
  char ch;

  printf("Enter a number: ");
  while((ch=getchar()) != '\n')
    n[ch-'0']++;
  printf("Digit:      ");
  for(i=0;i<10;i++)
    printf("%2d", i);
  printf("\nOccurrences:");
  for(i=0;i<10;i++)
    printf("%2d", n[i]);
  return 0;
}
