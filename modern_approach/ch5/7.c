#include<stdio.h>


int main(int argc, char *argv[])
{
  int i,j,k,q,mx1,mi1,mx2,mi2,max,min;

  printf("Enter four integers: ");
  scanf("%d %d %d %d", &i, &j, &k, &q);

  //归并
  mx1 = i;
  mi1 = j;

  if(i<j) {
    mx1 = j;
    mi1 = i;
  }

  mx2 = k;
  mi2 = q;

  if(k<q) {
    mx2 = q;
    mi2 = k;
  }

  if(mx2>mx1)
    max = mx2;
  else
    max = mx1;

  if(mi2>mi1)
    min = mi1;
  else
    min= mi2;



  printf("Largest: %d\nSmallest: %d",max, min);

  return 0;
}
