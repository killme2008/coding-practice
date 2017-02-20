#include <stdio.h>

int main(int argc, char *argv[])
{
  int w;
  printf("Enter the wind: ");
  scanf("%d", &w);

  if(w<1)
    printf("Calm\n");
  else if(w<4)
    printf("Light air\n");
  else if(w<28)
    printf("Breeze\n");
  else if(w<48)
    printf("Gale\n");
  else if(w<63)
    printf("Storm\n");
  else
    printf("Hurricane\n");

  return 0;
}
