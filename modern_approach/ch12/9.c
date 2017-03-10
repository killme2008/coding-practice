#include <stdio.h>

double inner_product(const double* a, const double* b, int n);

int main(int argc, char *argv[])
{
  double a[] = {1,2,3,4,5};

  printf("inner_product(a,a,5) = %g", inner_product(a,a,5));
  return 0;
}

double inner_product(const double* a, const double* b, int n) {
  const double *p, *q;

  double ret = 0.0;
  for(p = a, q = b; p < a+n; p++, q++)
    ret += *p * *q;
  return ret;
}
