#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

int roll_dice(void);
bool play_game(void);

int main(int argc, char *argv[])
{
  int wins, losses;
  char ch;

  wins = losses = 0;

  srand((unsigned)time(NULL));

  do {
    if(play_game())
      wins++;
    else
      losses++;

    printf("Play again?");
    ch = getchar();
    getchar();
  } while(ch == 'Y' || ch == 'y');


  printf("Wins: %d  Losses: %d\n", wins, losses);

  return 0;
}

int roll_dice(void) {
  return rand()%6 + rand()%6 + 2;
}

bool play_game(void) {
  bool first=true;
  int d;
  int o;

  for(;;) {
    d = roll_dice();
    printf("You rolled: %d\n", d);
    if(first) {
      if(d == 7 || d == 11){
        printf("You win!\n");
        return true;
      }
      first = false;
      printf("Your point is %d\n", d);
      o = d;
    } else {
      if(d == 7){
        printf("You lose!\n");
        return false;
      } else if(d == o) {
        printf("You win!\n");
        return true;
      }
    }
  }
}
