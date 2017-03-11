#include <stdio.h>
#include <string.h>
#define LEN 20

void read_word(char *s) {
  printf("Enter word:");
  scanf("%s", s);
}

int main(int argc, char *argv[])
{
  char smallest_word[LEN], largest_word[LEN], word[LEN];
  smallest_word[0] = largest_word[0] = '\0';

  for(;;){
    read_word(word);

    if(*smallest_word == '\0' || strcmp(word, smallest_word) < 0)
      strcpy(smallest_word, word);
    if(*largest_word == '\0' || strcmp(word, largest_word) > 0)
      strcpy(largest_word, word);
    if(strlen(word) == 4)
      break;
  }

  printf("Smallest word: %s\n", smallest_word);
  printf("Largest  word: %s\n", largest_word);

  return 0;
}
