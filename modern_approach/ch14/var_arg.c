#include <stdio.h>

#define TEST(condition, ...) ((condition) ? \
                              printf("passed test: %s\n", #condition): \
                              printf(__VA_ARGS__)) \

int main(int argc, char *argv[])
{
  int i, j;

  i = 1;
  j = 2;

  TEST(i < j, "i %d is equal to or greater than j %d", i, j);
  return 0;
}
