#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define NUM_PLANETS 9

int i_strcmp(const char *r, const char *t);

int main(int argc, char *argv[])
{
  char *planets[] = {
    "Mercury", "Venus", "Earth",
    "Mars", "Jupiter", "Saturn",
    "Uranus", "Neptune", "Pluto"
  };

  int i, j;

  for( i = 1; i < argc; i++) {
    for(j = 0; j < NUM_PLANETS; j++) {
      if(i_strcmp(argv[i], planets[j]) == 0) {
        printf("%s is planet %d\n", argv[i], j+1);
        break;
      }
    }
    if(j == NUM_PLANETS)
      printf("%s is not a planet\n", argv[i]);
  }

  return 0;
}

int i_strcmp(const char *s, const char *t) {
  const char *p, *q;
  for(p = s, q = t; toupper(*p) == toupper(*q); p++, q++)
    if(*p == '\0')
      return 0;
  return toupper(*p) - toupper(*q);
}
