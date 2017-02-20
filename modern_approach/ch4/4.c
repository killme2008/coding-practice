#include<stdio.h>

int main(int argc, char *argv[])
{
  int d;
  printf("Enter a number between 0 and 32767: ");
  scanf("%d", &d);
  printf("In octal, your number is: 0%o", d);
  return 0;
}
