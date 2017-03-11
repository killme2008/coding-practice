#include <stdio.h>

int my_strcmp(char *s, char *t) {
  char *p, *q;
  for(p = s, q = t; *p == *q; p++, q++)
    if(*p == '\0')
      return 0;
  return *p - *q;
}

int main(int argc, char *argv[])
{
  printf("%d\n", my_strcmp("hello", "hellw"));
  printf("%d\n", my_strcmp("hell", "hellw"));
  printf("%d\n", my_strcmp("hello world", "hello"));
  return 0;
}
