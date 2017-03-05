#include <stdio.h>
#define N 10

void min_max(int a[], int n, int* max, int* min);

int main(int argc, char *argv[])
{
  int a[N], small, big, i;
  printf("Enter numbers (at most 10):");
  for(i = 0; i < N; i++)
    scanf("%d", &a[i]);
  min_max(a, N, &big, &small);

  printf("Largest : %d\n", big);
  printf("Smallest: %d\n", small);
  return 0;
}

void min_max(int a[], int n, int* max, int* min) {
  int i;

  *max = *min = a[0];
  for(i = 1; i < n; i++){
    if(*max < a[i])
      *max = a[i];
    else if(*min > a[i])
      *min = a[i];
  }
}
