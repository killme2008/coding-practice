#include <stdio.h>
#include <string.h>

#define TOUPPER(c) ('a' <= (c) && (c) <= 'z' ? (c) - 'a' + 'A' : (c))

int main(int argc, char *argv[])
{
  int i;
  char s[10];

  strcpy(s, "abcd");
  i = 0;
  putchar(TOUPPER(s[++i]));

  strcpy(s, "0123");
  i = 0;
  putchar(TOUPPER(s[++i]));

  return 0;
}
