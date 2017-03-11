#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#define LOOP 10000000

size_t strlen1(const char *s) {
  size_t n = 0;
  while(*s++)
    n++;
  return n;
}

size_t strlen2(const char *s) {
  const char* p = s;

  while(*s)
    s++;
  return s - p;
}

void benchmark(size_t(*len)(const char*)) {
  int i, j = 0;
  char s[1000];
  for(i = 0; i< 999; i++)
    s[i] = 'a';
  s[999] = '\0';


  time_t start = time(NULL);
  for(i = 0; i < LOOP; i++)
    j += len(s);

  time_t end = time(NULL);
  int cost =  (uintmax_t)end - (uintmax_t)start;

  printf("%d %d secs.\n", j, cost);
}

char *mstrcat(char *s1, const char *s2) {
  char *p = s1;
  while(*p)
    p++; //reach end

  while((*p++ = *s2++))
    ;

  return s1;
}

int main(int argc, char *argv[])
{

  //22 seconds，并没有更快
  //Apple LLVM version 8.0.0 (clang-800.0.42.1)
  //benchmark(&strlen2);
  //19 seconds
  //benchmark(&strlen1);

  char s1[] = "hello";
  char s2[] = " world";

  printf("%s", mstrcat(s1, s2));

  return 0;
}
