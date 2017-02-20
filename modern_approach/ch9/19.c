#include <stdio.h>

void pb(int n) {
  if(n>0) {
    pb(n/2);
    putchar('0' + n%2);
  }
}

int main(int argc, char *argv[])
{
  int d;
  printf("Enter a number:");
  scanf("%d", &d);
  pb(d);
  return 0;
}
