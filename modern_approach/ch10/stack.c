#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define STACK_SIZE 100

int contents[STACK_SIZE];
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
  printf("stack overflow.");
  exit(1);
}

void stack_underflow(void) {
  printf("stack underflow.");
  exit(1);
}

void push(int i) {
  if(is_full())
    stack_overflow();
  else
    contents[top++] = i;
}

int pop(void) {
  if(is_empty())
    stack_underflow();
  else
    return contents[--top];
}

int main(int argc, char *argv[])
{
  for(int i = 0; i < 10; i++){
    push(i);
  }

  for(int i = 0; i < 10; i++){
    printf("%d ",pop());
  }

  return 0;
}
