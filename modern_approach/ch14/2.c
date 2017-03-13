#include <stdio.h>
#include "global.h"

#define NELEMS(a) (sizeof((a))/sizeof((a)[0]))

int main(int argc, char *argv[])
{
  int a[10], b[100];
  PRINT_INT(NELEMS(a));
  PRINT_INT(NELEMS(b));
  return 0;
}
