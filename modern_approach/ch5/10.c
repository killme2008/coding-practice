#include<stdio.h>

int main(int argc, char *argv[])
{
  int d;
  printf("Enter numberic grade: ");
  scanf("%d", &d);

  if(d<0 || d>100){
    printf("Bad value.\n");
    return 1;
  }

  switch(d/10) {
  case 10: case 9:
    printf("Letter grade: A");
    break;
  case 8:
    printf("Letter grade: B");
    break;
  case 7:
    printf("Letter grade: C");
    break;
  case 6:
    printf("Letter grade: D");
    break;
  default:
    printf("Letter grade: F");
    break;
  }
  return 0;
}
