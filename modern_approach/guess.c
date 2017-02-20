#include <stdio.h>
#include <time.h>
#include <stdlib.h>

 void init_random_n(void);
int generate_guess_n(void);
void guess(int n);

int main(int argc, char *argv[])
{
  int n;
  char ch;
  init_random_n();

  printf("I have a number between 1 and 100, guess it? Enter 'Y' or 'y' to start.");
  ch = getchar();
  while(ch == 'Y' || ch == 'y'){
    n = generate_guess_n();
    guess(n);
    printf("I have a number between 1 and 100, guess it? Enter 'Y' or 'y' to start.");
    ch = getchar();
    printf("%c\n", ch);
  }

  return 0;
}
void init_random_n(void) {
  srand((unsigned)time(NULL));
}
int generate_guess_n(void) {
  return rand() % 100 + 1;
}
void guess(int n) {
  int d, guessed;
  guessed = 0;
  for(;;){
    guessed++;
    printf("I guess: ");
    scanf("%d", &d);
    if(d == n) {
      printf("You guessed %d times to got %d!\n", guessed, d);
      return;
    } else if(d > n) {
      printf("Too high, try again.\n");
    } else {
      printf("Too low, try again.\n");
    }
  }
}
