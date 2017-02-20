#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#define N 26

int main(int argc, char *argv[])
{
  char ch;
  char a[N];

  for(;;) {
    for(int i = 0; i < N; i++)
      a[i] = 0;

    printf("Enter first word: ");
    while((ch = getchar()) != '\n'){
      if(isalpha(ch))
        a[tolower(ch) - 'a']++;
    }
    printf("Enter second word: ");
    while((ch = getchar()) != '\n'){
      if(isalpha(ch))
        a[tolower(ch) - 'a']--;
    }

    bool anagram = true;
    for(int i = 0; i < N; i++)
      if(a[i] !=0) {
        anagram = false;
        break;
      }

    if(anagram)
      printf("The words are anagrams.\n");
    else
      printf("The words are not anagrams.\n");
  }

  return 0;
}
