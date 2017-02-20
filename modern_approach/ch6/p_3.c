#include <stdio.h>

static int gcd(int m, int n) {
  int r;
  if(n==0)
    return m;
  return gcd(n, m % n);
}

int main(int argc, char *argv[])
{
  int f, n, g;
  printf("Enter a fraction: ");
  scanf("%d/%d", &f, &n);
  g = gcd(f, n);
  printf("In lowest terms:%d/%d\n" ,f/g, n/g);
  return 0;
}
