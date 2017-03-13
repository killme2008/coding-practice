#include <stdio.h>

#define PRINT_INT(n) printf(#n" = %d\n", n)
#define PRINT_STR(s) printf(#s" = %s\n", s)

int main(int argc, char *argv[])
{
  PRINT_INT(__LINE__);
  PRINT_STR(__FILE__);
  PRINT_STR(__DATE__);
  PRINT_STR(__TIME__);
  PRINT_INT(__STDC__);
  PRINT_INT(__STDC_VERSION__);
  PRINT_INT(__STDC_HOSTED__);
  return 0;
}
