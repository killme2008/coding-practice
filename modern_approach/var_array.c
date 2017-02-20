#include <stdio.h>

unsigned sum(int n, int a[n]) {
  unsigned sum = 0;
  for(int i  = 0; i < n; i++)
    sum += a[i];
  return sum;
}

int main(int argc, char *argv[])
{
  int n;
  printf("Enter a number: ");
  scanf("%d", &n);
  int a[n];

  for(int i=0; i<n; i++)
    a[i] = i+1;
  printf("Sum: %u", sum(n,a));

  return 0;
}
