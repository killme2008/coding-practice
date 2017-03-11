#include <stdio.h>

void censor(char s[]);

int main(int argc, char *argv[])
{
  char s1[] = " food fool";
  censor(s1);
  puts(s1);
  char s2[] = "foofool fuck fo";
  censor(s2);
  puts(s2);
  char s3[] = "fooofood fox";
  censor(s3);
  puts(s3);
  return 0;
}

void censor(char s[]) {
  char *p;
  while(*s){
    if(*s == 'f') {
      p = s;
      while(*++s == 'o')
        ;
      if(s-p >= 2){
        *p = *(p+1) = *(p+2) = 'x';
      }
    } else
      s++;
  }
}
