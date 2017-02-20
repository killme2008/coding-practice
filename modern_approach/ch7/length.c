#include <stdio.h>

int main(int argc, char *argv[])
{
  int len=0;

  printf("Enter a message: ");
  while(getchar() != '\n')
    len++;
  printf("Your message was %d characters long.\n", len);
  return 0;
}
