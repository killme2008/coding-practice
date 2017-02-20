#include <stdio.h>
#define N 10
void qsort(int a[], int low, int high);
int split(int a [], int low, int high);
int main(int argc, char *argv[])
{
  int a[N];
  int i, low, high;
  printf("Enter 10 numbers to sort: ");
  for(i = 0; i < N; i++)
    scanf("%d", &a[i]);
  qsort(a, 0, N-1);
  printf("Sorted numbers:");
  for(i = 0; i < N; i++)
    printf(" %d", a[i]);
  return 0;
}

void qsort(int a[], int low, int high) {
  if(low>=high)
    return;
  int chosen = split(a, low, high);
  qsort(a, low, chosen-1);
  qsort(a, chosen+1, high);
}

int split(int a[], int low, int high){
  int v = a[low];
  for(;;){
    while(low<high && a[high] > v)
      high--;
    if(low>=high)
      break;
    a[low++] = a[high];

    while(low<high && a[low] < v)
      low++;
    if(low>=high)
      break;
    a[high--] = a[low];
  }
  a[high] = v;
  return high;
}
