#include <stdio.h>
#include "global.h"

#define IN(x, n) ((x) >=0 && (x) <= ((n) - 1))
#define CHECK(x,y,n) (IN((x), (n)) && IN((y), (n)) ? 1 : 0)
#define M(x,y,z) ((((x)-(y)) * ((x) - (z))) <= 0)
#define MEDIAN(x,y,z) (M(x, y, z) ? (x) : (M(y,x,z) ? (y) : (z)))
#define POLYNOMIAL(x) ( (3 * ((x) * (x) * (x) * (x) * (x)))         \
                       +(2 * ((x) * (x) * (x) * (x)))               \
                       -(5 * ((x) * (x) * (x)))                     \
                       -((x) * (x)) + (7 * (x)) - 6 )



int main(int argc, char *argv[])
{
  PRINT_INT(CHECK(1,2,3));
  PRINT_INT(CHECK(1,3,3));

  PRINT_INT(MEDIAN(1,2,3));
  PRINT_INT(MEDIAN(3,1,2));
  PRINT_INT(MEDIAN(2,3,1));
  PRINT_INT(MEDIAN(1,3,2));
  PRINT_INT(POLYNOMIAL(10));
  return 0;
}
