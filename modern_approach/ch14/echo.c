#include <stdio.h>

#define ECHO(x) \
  do {          \
    gets(s);    \
    puts(s);    \
  }while(0)     \

int main(int argc, char *argv[])
{
  char s[100];
  if(argc > 1)
    ECHO(s);
  else
    puts("none.");

  return 0;
}
