#include <stdio.h>
#include <stdbool.h>

int temperatures[7][24] = {
  {1},
  {2},
  {3},
  {4},
  {5, 32},
  {6},
  {7},
};

bool search(int a[], int n, int key);

int main(int argc, char *argv[])
{
  // temperatures[0] type is int*
  printf("%d", search(temperatures[0], 24*7, 32));
  return 0;
}

bool search(int a[], int n, int key) {
  int *p;
  for(p = a; p < a + n; p++)
    if(*p == key)
      return true;
  return false;
}
