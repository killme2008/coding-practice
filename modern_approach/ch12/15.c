#include <stdio.h>

int temperatures[7][24] = {
  {1},
  {2},
  {3},
  {4},
  {5, 32},
  {6},
  {7},
};

void print_row(int a[][24], int i);

int main(int argc, char *argv[])
{
  print_row(temperatures, 3);
  print_row(temperatures, 4);
  return 0;
}

void print_row(int a[][24], int i) {
  int *p;
  for(p = a[i]; p < a[i] + 24; p++)
    printf("%d ", *p);
  printf("\n");
}
