#include <stdio.h>

float compute_average_word_length(const char *sentence);

int main(int argc, char *argv[])
{

  printf("%.2f\n", compute_average_word_length("dennis zhuang"));
  printf("%.2f\n", compute_average_word_length("hello"));
  printf("%.2f\n", compute_average_word_length("hello the c programming language"));
  return 0;
}

float compute_average_word_length(const char *sentence) {
  double c, n;
  const char *p, *q;

  c = n = 0;
  p = q = sentence;

  while(*p) {
    if(*p == ' ') {
      c += p - q;
      n++;
      while(*p == ' ')
        p++;
      q = p;
    }
    p++;
  }
  c += p - q;
  n++;

  return c/n;
}
