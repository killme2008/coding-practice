#include "quicksort.h"

void quick_sort(int a[], int low, int high) {
  if(low>=high)
    return;
  int chosen = split(a, low, high);
  quick_sort(a, low, chosen-1);
  quick_sort(a, chosen+1, high);
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
