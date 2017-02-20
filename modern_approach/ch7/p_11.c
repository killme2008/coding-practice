#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
  char ch, f;

  printf("Enter a first and last name: ");
  while((ch = getchar()) == ' ')
    ;

  f = ch;
  while((ch = getchar()) != ' ')
    ;
  while((ch = getchar()) == ' ')
    ;

  putchar(ch);
  while((ch = getchar()) != '\n') {
    if(ch != ' ')
      putchar(ch);
  }
  putchar(',');
  putchar(' ');
  putchar(toupper(f));

  return 0;
}
