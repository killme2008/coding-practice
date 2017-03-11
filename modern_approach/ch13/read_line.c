#include <stdio.h>

int read_line(char str[], int n) {
  int ch, i = 0;

  while((ch = getchar()) != '\n') {
    if(i<n)
      str[i++] = ch;
  }

  str[i] = '\0';
  return i;
}

int main(int argc, char *argv[])
{

  printf("Enter a line:");
  char s[100];
  printf("Readed %d characters.", read_line(s, 100));
  printf("The line is:%s", s);
  return 0;
}
