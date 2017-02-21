#include <stdio.h>

int calc(int x);

int main(int argc, char *argv[])
{
  int d;
  printf("Enter a number:");
  scanf("%d", &d);
  printf("Result = %d\n", calc(d));
  return 0;
}

int mpow(int x, int n) {
  int m = 1;
  for(int i=0; i<n; i++)
    m*=x;
  return m;
}

int calc(int x) {
  return 3*mpow(x, 5) + 2*mpow(x, 4) - 5*mpow(x, 3) -
    mpow(x, 2) + 7*x - 6;
}
