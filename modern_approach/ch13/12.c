#include <stdio.h>
#include <string.h>

void get_extension(const char *file_name, char *ext);

int main(int argc, char *argv[])
{
  char s1[] = "hello.txt";
  char e1[4];
  get_extension(s1, e1);
  puts(e1);

  char s2[] = "hello";
  char e2[4];
  get_extension(s2, e2);
  puts(e2);

  char s3[] = "hello.txt.txt";
  char e3[4];
  get_extension(s3, e3);
  puts(e3);

  return 0;
}

void get_extension(const char *f, char *e) {
  const char *p;
  p = f + strlen(f);
  while(p > f) {
    if(*p == '.')
      break;
    p--;
  }
  if(p == f)
    *e = '\0';
  else
    stpcpy(e, ++p);
}
