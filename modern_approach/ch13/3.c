#include <stdio.h>

int main(int argc, char *argv[])
{
  int i, j;
  char s[10];
  //Input "12abc34 56def78"
  scanf("%d%s%d", &i, s, &j);

  printf("%d %d '%s'", i, j , s);
  return 0;
}
