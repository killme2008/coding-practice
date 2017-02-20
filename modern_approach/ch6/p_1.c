#include <stdio.h>

int main(int argc, char *argv[])
{
  float f, max;
  max = 0.0f;
  for(;;) {
    printf("Enter a number: ");
    scanf("%f", &f);
    if(f<=0){
      printf("The largest number entered was %.2f\n" ,max);
      break;
    }

    if(f > max)
      max = f;
  }
  return 0;
}
