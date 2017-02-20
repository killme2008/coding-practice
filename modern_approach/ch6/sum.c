#include<stdio.h>

int main(int argc, char *argv[])
{
  int i,n;

  printf("This program sums a series of integers.\n");
  printf("Enter integers( 0 to terminate): ");

  scanf("%d", &i);
  while(i!=0){
    n += i;
    scanf("%d", &i);
  }

  printf("Sum: %d\n", n);
  return 0;
}
