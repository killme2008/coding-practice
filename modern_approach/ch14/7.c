#include <stdio.h>

#define GENERIC_MAX(type)           \
  type type##_max(type x, type y) { \
    return x > y ? x : y;           \
  }                                 \

typedef unsigned long UL;

GENERIC_MAX(UL)

int main(int argc, char *argv[]){
  UL i, j;
  i = 100;
  j = 200;
  printf("%lu",UL_max(i,j));
  return 0;
}
