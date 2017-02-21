#include <stdio.h>
#define N 10

void selection_sort(int a[], int m);

int main(int argc, char *argv[])
{
  int i, a[N];
  printf("Enter 10 numbers to sort:");
  for(i = 0; i < N; i++)
    scanf("%d", &a[i]);

  selection_sort(a, N);

  printf("Sorted numbers:");
  for(i = 0; i < N; i++)
    printf(" %d", a[i]);
  printf("\n");

  return 0;
}

void selection_sort(int a[], int m) {
  if(m == 0)
    return;
  int max = 0;
  for(int i = 1; i < m; i++) {
    if(a[i] > a[max]) {
      max = i;
    }
  }
  int temp = a[max];
  a[max] = a[m-1];
  a[m-1] = temp;
  selection_sort(a, m-1);
}
