#include <stdio.h>

int main(int argc, char *argv[])
{
  int i, j, n;
  double f, r = 1.0f;
  printf("Enter interge n: ");
  scanf("%d", &n);
  for(i=1;i<=n;i++) {
    for(f=1.0f, j=1;j<=i;j++)
      f *= j;
    r += 1/f;
  }

  printf("e=%.20f\n", r);
  return 0;
}
