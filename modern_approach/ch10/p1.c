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
  printf("stack overflow.");
  exit(1);
}

void stack_underflow(void) {
  printf("stack underflow.");
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

  while((ch = getchar()) != '\n') {
    switch(ch) {
    case '(': case '{':
      push(ch);
      break;
    case ')':
      if(pop() != '(') {
        printf("Parenteses are not nested properly\n");
        exit(1);
      }
      break;
    case '}':
      if(pop() != '{') {
        printf("Braces are not nested properly\n");
        exit(1);
      }
      break;
    default:
      printf("unexpect char '%c'.", ch);
      exit(1);
    }
  }

  if(!is_empty()) {
    printf("Parenteses/braces are not nested properly\n");
    return 1;
  } else {
    printf("Parenteses/braces are nested properly\n");
    return 0;
  }
}
