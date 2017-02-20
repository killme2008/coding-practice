#include<stdio.h>

int main(int argc, char *argv[])
{
  int d;
  printf("Enter a three-digit number: ");
  scanf("%d", &d);
  printf("The reverse is: %d%d%d", d%100%10, d/10%10, d/100);
  return 0;
}
