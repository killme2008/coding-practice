#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define N 26

bool are_anagrams(const char *w1, const char *w2);

int main(int argc, char *argv[])
{

  printf("%d\n", are_anagrams("hello", "LOLEH"));
  printf("%d\n", are_anagrams("hello", "helle"));
  return 0;
}

bool are_anagrams(const char *w1, const char *w2) {
  int a[N] = {0};

  const char *p = w1;
  while(*p) {
    a[tolower(*p) - 'a']++;
    p++;
  }

  p = w2;
  while(*p) {
    a[tolower(*p) - 'a']--;
    p++;
  }

  for(int i = 0; i < N; i++)
    if(a[i]) {
      return false;
    }
  return true;

}
