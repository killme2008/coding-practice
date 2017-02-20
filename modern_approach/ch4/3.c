#include<stdio.h>

int main(int argc, char *argv[])
{
  int d1,d2,d3;
  printf("Enter a three-digit number: ");
  scanf("%1d%1d%1d", &d1, &d2, &d3);
  printf("The reverse is: %d%d%d", d3, d2, d1);
  return 0;
}
