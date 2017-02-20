#include <stdio.h>

unsigned long long int fact(int);
int main(int argc, char *argv[])
{
  int n;
  printf("Enter a number:");
  scanf("%d", &n);
  printf("%d! = %llu\n", n, fact(n));
  return 0;
}
unsigned long long int fact(int n) {
  unsigned long long int f = 1;
  for(int i=2;i<=n;i++)
    f *= i;
  return f;
}
