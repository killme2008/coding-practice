#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
  int n = 0;
  char ch;

  printf("Enter a sentence:");

  while((ch = getchar()) != '\n') {
    switch(tolower(ch)){
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
      n++;
      break;
    default:
      break;
    }
  }

  printf("Your sentence contains %d vowels.\n", n);

  return 0;
}
