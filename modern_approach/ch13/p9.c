#include <stdio.h>

int compute_vowel_count(const char* sentence);

int main(int argc, char *argv[])
{
  printf("%d\n", compute_vowel_count("the c programming language."));

  return 0;
}

int compute_vowel_count(const char* sentence) {
  int c = 0;
  while(*sentence) {
    switch(*sentence) {
    case 'a': case 'e': case 'i': case 'o': case 'u':
      c++;
      break;
    }
    sentence++;
  }
  return c;
}
