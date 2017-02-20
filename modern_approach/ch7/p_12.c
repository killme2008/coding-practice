#include <stdio.h>

int main(int argc, char *argv[])
{
  double d, r;
  char op;

  printf("Enter an expression: ");

  scanf("%lf", &d);
  r = d;

  for(;;){
    op = getchar();
    if(op == '\n')
      break;
    scanf("%lf", &d);
    switch(op){
    case '+':
      r += d;
      break;
    case '-':
      r -= d;
      break;
    case '*':
      r *= d;
      break;
    case '/':
      r /= d;
      break;
    default:
      printf("Invalid expression.");
      return 1;
    }
  }

  printf("Value of expression: %.2lf", r);

  return 0;
}
