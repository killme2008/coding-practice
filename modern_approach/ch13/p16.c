#include <stdio.h>
#include <string.h>

void reverse(char *message);

int main(int argc, char *argv[])
{
  char s[] = "hello world";
  reverse(s);

  printf("%s", s);
  return 0;
}

void reverse(char *message) {
  char *p, *q, c;
  p = message;
  q = message + strlen(message) - 1;

  while(p < q){
    c = *p;
    *p = *q;
    *q = c;
    p++;
    q--;
  }
}
