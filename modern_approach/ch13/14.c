#include <stdio.h>


int main(int argc, char *argv[])
{
  char s[] = "Hsjodi", *p;
  for(p = s; *p; p++)
    --*p;
  puts(s);

  return 0;
}
