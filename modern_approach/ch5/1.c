#include<stdio.h>

int main(int argc, char *argv[])
{
  int d,ds;

  printf("Enter a number: ");
  scanf("%d", &d);

  if(d>=1000)
    ds = 4;
  else if(d>=100)
    ds = 3;
  else if(d>=10)
    ds = 2;
  else
    ds = 1;
  printf("The number %d has %d digits.", d, ds);

  return 0;
}
