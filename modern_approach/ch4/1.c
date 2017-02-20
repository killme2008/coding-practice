#include<stdio.h>

int main(int argc, char *argv[])
{
  int d;
  printf("Enter a two-digit number: ");
  scanf("%d", &d);
  printf("The reverse is: %d%d", d%10, d/10);
  return 0;
}
