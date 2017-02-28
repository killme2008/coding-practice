#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define STACK_SIZE 100

char contents[STACK_SIZE];
int top = 0;

void make_empty(void) {
  top = 0;
}

bool is_empty(void) {
  return top == 0;
}

bool is_full(void) {
  return top == STACK_SIZE;
}

void stack_overflow(void) {
  printf("Expression is too complex.");
  exit(1);
}

void stack_underflow(void) {
  printf("Not enough operands in expression.");
  exit(1);
}

void push(char i) {
  if(is_full())
    stack_overflow();
  else
    contents[top++] = i;
}

char pop(void) {
  if(is_empty())
    stack_underflow();
  else
    return contents[--top];
}

int main(int argc, char *argv[])
{
  char ch;
  int a, b;

 start:
  for(;;) {
    printf("Enter an RPN expression:");
    for(;;){
      scanf(" %c", &ch);
      if(ch >= '0' && ch <= '9') {
        push(ch - '0');
      } else {
        switch(ch) {
        case '+':
          b = pop();
          a = pop();
          push(a + b);
          break;
        case '-':
          b = pop();
          a = pop();
          push(a - b);
          break;
        case '*':
          b = pop();
          a = pop();
          push(a * b);
          break;
        case '/':
          b = pop();
          a = pop();
          push(a / b);
          break;
        case '=':
          printf("value of expression: %d\n", pop());
          while(!is_empty())
            pop();
          goto start;
        default:
          return 0;
        }
      }
    }
  }
}
