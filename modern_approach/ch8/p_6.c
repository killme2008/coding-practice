#include <stdio.h>
#include <ctype.h>
#define MSG_LEN 100

int main(int argc, char *argv[])
{
  char msg[MSG_LEN];
  char ch;
  int i, n;
  i = n = 0;

  printf("Enter message: ");
  while((ch = getchar()) != '\n')
    msg[n++] = ch;

  if(n > MSG_LEN){
    printf("Too long msg.");
    return 1;
  }

  printf("In B1FF-Speek: ");
  for(i = 0; i < n; i++){
    ch = toupper(msg[i]);
    switch(ch){
    case 'A':
      printf("4");
      break;
    case 'B':
      printf("8");
      break;
    case 'E':
      printf("3");
      break;
    case 'I':
      printf("1");
      break;
    case 'O':
      printf("0");
      break;
    case 'S':
      printf("5");
      break;
    default:
      printf("%c", ch);
      break;
    }
  }
  for(int i=0;i<10;i++)
    printf("!");
  return 0;
}
