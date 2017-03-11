#include <stdio.h>
#include <ctype.h>

void capitalize1(char s[]);
void capitalize2(char s[]);

int main(int argc, char *argv[])
{
  char s[] = "hello world";
  capitalize1(s);
  //capitalize2(s);
  puts(s);
  return 0;
}

void capitalize1(char s[]) {
  while(*s) {
    *s = toupper(*s);
    s++;
  }
}
void capitalize2(char s[]) {
  int i;
  for(i = 0; s[i] != '\0'; i++)
    s[i]= toupper(s[i]);
}
