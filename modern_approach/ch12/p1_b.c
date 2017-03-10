#include <stdio.h>
#define LEN 100

int main(int argc, char *argv[])
{
  char msg[LEN];
  int ch;
  char *p;

  p = msg;

  printf("Enter a message:");

  while((ch = getchar()) != '\n')
    *p++ = ch;

  printf("Reversal is:");
  while(p >= msg)
    putchar(*--p);

  return 0;
}
