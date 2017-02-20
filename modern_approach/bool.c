#include <stdbool.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  bool flag = true;
  printf("%d %d\n", flag? 1 : 0, flag);
  return 0;
}
