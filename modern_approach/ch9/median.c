#include <stdio.h>

double median(double, double, double);

int main(int argc, char *argv[])
{
  printf("median(1.0, 2.0, 3.0) = %g\n", median(1.0, 2.0, 3.0));
  printf("median(1.0, 3.0, 2.0) = %g\n", median(1.0, 3.0, 2.0));
  printf("median(2.0, 1.0, 3.0) = %g\n", median(2.0, 1.0, 3.0));
  printf("median(1.0, 1.0, 2.0) = %g\n", median(1.0, 1.0, 2.0));
  printf("median(1.0, 2.0, 1.0) = %g\n", median(1.0, 2.0, 1.0));
  printf("median(2.0, 1.0, 1.0) = %g\n", median(2.0, 1.0, 1.0));
  return 0;
}

double median(double x, double y, double z) {
  double a = x-y;
  double b = y-z;
  double c = x-z;
  if(a*b > 0) return y;
  if(a*c > 0) return z;
  return x;
}
