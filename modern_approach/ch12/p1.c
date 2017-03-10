#include <stdio.h>
#define LEN 100

int main(int argc, char *argv[])
{
  char msg[LEN];
  int ch, i;
  i = 0;

  printf("Enter a message:");

  while((ch = getchar()) != '\n')
    msg[i++] = ch;

  printf("Reversal is:");
  for(int j = i-1; j >= 0; j--)
    putchar(msg[j]);

  return 0;
}
