#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
  double y1, y2;
  int x;

  double c = 0.00001;
  y1=0.0;
  y2=1.0;

  printf("Enter a postive number: ");
  scanf("%d", &x);

  while(fabs(y2-y1) >= y2*c){
    y1 = y2;
    y2 = (y1 + x/y1)/2;
  }

  printf("Square root of %d is: %lf\n", x, y2);

  return 0;
}
