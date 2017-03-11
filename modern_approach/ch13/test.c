#include <stdio.h>

int main(int argc, char *argv[])
{
  char *p = "test";
  //modify literal : bus error
  //跟数组不同
  *p = 'd';

  printf("%s", p);
  return 0;
}
