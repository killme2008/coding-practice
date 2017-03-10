#include <stdio.h>
#define LEN 10

int sum_two_diamensional_array(const int a[][LEN], int n);

int main(int argc, char *argv[])
{

  int temperatures[10][LEN] = {
    {1},
    {2},
    {3},
    {4},
    {5},
    {6},
    {7},
    {8},
    {9},
    {10},
  };

  printf("sum(temperatures) = %d",
         sum_two_diamensional_array(temperatures, 10));

  return 0;
}

int sum_two_diamensional_array(const int a[][LEN], int n) {
  int sum = 0;
  const int *p;
  for(p = &a[0][0]; p <= &a[n-1][LEN-1] ; p++)
    sum += *p;
  return sum;
}
