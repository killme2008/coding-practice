#include <stdio.h>

int check(int, int, int);
int main(int argc, char *argv[])
{

  printf("check(1, 2, 3) = %d\n", check(1, 2, 3));
  printf("check(3, 2, 1) = %d\n", check(3, 2, 1));
  return 0;
}

int check(int x, int y, int n) {
  if(x >=0 && x <= n && y >=0 && y<= n)
    return 1;
  else
    return 0;
}
