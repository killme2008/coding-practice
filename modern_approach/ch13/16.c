#include <stdio.h>

int count_spaces(const char s[]) {
  int c = 0;
  while(*s){
    if(*s == ' ') c++;
    s++;
  }
  return c;
}

int main(int argc, char *argv[])
{
  printf("%d", count_spaces("hello, the c programming language."));
  return 0;
}
