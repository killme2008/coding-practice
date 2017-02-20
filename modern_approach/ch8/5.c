#include <stdio.h>

int main(int argc, char *argv[])
{
  int n, i;
  printf("Enter a number: ");
  scanf("%d", &n);

  if(n<2){
    printf("Invalid number.");
    return 1;
  }
  unsigned long long a[n]; //c99 only.

  a[0] = 0;
  a[1] = 1;
  for(i=2;i<n;i++) {
    a[i] = a[i-1] + a[i-2];
  }

  printf("fib numbers:");
  for(i=0;i<n;i++) {
    printf(" %llu", a[i]);
  }

  return 0;
}
