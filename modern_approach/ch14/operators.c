#include <stdio.h>

#define PRINT_INT(n) printf(#n" = %d\n", n)

#define GENERIC_MAX(type)           \
  type type##_max(type x, type y) { \
    return x > y ? x : y;           \
  }                                 \

GENERIC_MAX(float)
GENERIC_MAX(int)

int main(int argc, char *argv[])
{
  int i = 10;
  PRINT_INT(i);
  PRINT_INT(int_max(i, 11));

  return 0;
}
