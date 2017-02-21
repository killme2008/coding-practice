#include <stdio.h>

long power(int x, int n);

int main(int argc, char *argv[])
{
  int x, n;
  printf("Enter two numbers:");
  scanf("%d", &x);
  scanf("%d", &n);
  printf("power(%d, %d) = %ld\n", x, n, power(x, n));

  return 0;
}

long power(int x, int n) {
  if(n==0)
    return 1L;

  if(n%2==0){
    long t = power(x, n/2);
    return t*t;
  }
  else
    return ((long)x)*power(x, n-1);
}
