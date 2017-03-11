#include <stdio.h>

int f(char *s, char *t) {
  char *p1, *p2;

  for(p1 = s; *p1; p1++){
    for(p2 = t; *p2; p2++)
      if(*p1 == *p2) break;
    if( *p2 == '\0') break;
  }
  printf("%c\n", *p1);
  return p1 - s;
}

int main(int argc, char *argv[])
{
  printf("%d\n", f("abcd", "babc"));
  printf("%d\n", f("abcd", "bcd"));
  return 0;
}
