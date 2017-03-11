#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

bool test_extension(const char *file_name, const char* extension);

int main(int argc, char *argv[])
{
  printf("%d\n", test_extension("hello.txt", "TXT"));
  printf("%d\n", test_extension("hello.txt", "txt"));
  printf("%d\n", test_extension("hello.TXT", "txt"));
  printf("%d\n", test_extension("hello.tx", "TXT"));
  printf("%d\n", test_extension("hello.txt", "TX"));
  printf("%d\n", test_extension("hello", "TX"));
  return 0;
}

bool test_extension(const char *file_name, const char *extension) {
  const char *p;
  p = file_name;
  while(*p && *p != '.')
    p++;
  if(!*p)
    return false;
  p++;
  while(toupper(*p) == toupper(*extension)) {
    p++;
    extension++;
    if(*p == '\0') {
      return *extension == '\0';
    }
  }

  return false;
}
