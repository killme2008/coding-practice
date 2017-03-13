#include <stdio.h>
#include "global.h"

#define CUBIC(x) ((x) * (x) * (x)) //fail on side effect.
#define DIV4(x) ((x)/4)   //fail on float
#define PROD(x,y) ((x) * (y) < 100 ? 1 : 0) //fail on non-numeric types

int main(int argc, char *argv[])
{
  PRINT_INT(CUBIC(3));
  int i = 3;
  PRINT_INT(CUBIC(i++));

  PRINT_INT(DIV4(10));
  PRINT_FLOAT(DIV4(10.0));
  PRINT_INT(PROD(1,2));
  PRINT_INT(PROD(100,2));
  //PRINT_INT(PROD("test",2));
  return 0;
}
