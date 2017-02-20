#include <stdio.h>

int main(int argc, char *argv[])
{
  int i=9384;
  do
    {
      printf("%d\n", i);
      i /= 10;
    } while (i > 0);
  return 0;
}
