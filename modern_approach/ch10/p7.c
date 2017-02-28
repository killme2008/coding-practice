#include <stdio.h>
#include <ctype.h>

#define MAX_DIGITS 10

const int segments[10][7] = {
  {1, 1, 1, 1, 1, 1 ,0}, /* 0 */
  {0, 1, 1, 0, 0, 0 ,0}, /* 1 */
  {1, 1, 0, 1, 1, 0 ,1}, /* 2 */
  {1, 1, 1, 1, 0, 0 ,1}, /* 3 */
  {0, 1, 1, 0, 0, 1 ,1}, /* 4 */
  {1, 0, 1, 1, 0, 1 ,1}, /* 5 */
  {1, 0, 1, 1, 1, 1 ,1}, /* 6 */
  {1, 1, 1, 0, 0, 0 ,0}, /* 7 */
  {1, 1, 1, 1, 1, 1 ,1}, /* 8 */
  {1, 1, 1, 1, 0, 1 ,1}  /* 9 */
};

char digits[3][MAX_DIGITS * 4];

void clear_digits_array(void);
void process_digit(int digit, int position);
void print_digits_array(void);

int main(int argc, char *argv[])
{
  char ch;
  int position = 0;

  clear_digits_array();
  printf("Enter a number (up to 10 digits): ");
  while ((ch = getchar()) != '\n' && position < 4*MAX_DIGITS) {

    if (isdigit(ch)) {
      process_digit(ch - '0', position);
      position += 4;
    }
  }
  print_digits_array();
  return 0;
}

void clear_digits_array(void) {
  int i, j;

  for(i = 0; i < 3; i++) {
    for(j = 0; j < MAX_DIGITS*4; j++)
      digits[i][j] = ' ';
  }
}

void process_digit(int digit, int pos) {

  if(segments[digit][0]) {
    digits[0][pos+1] = '_';
  }

  if(segments[digit][1]) {
    digits[1][pos+2] = '|';
  }

  if(segments[digit][2]) {
    digits[2][pos+2] = '|';
  }

  if(segments[digit][3]) {
    digits[2][pos+1] = '_';
  }

  if(segments[digit][4]) {
    digits[2][pos] = '|';
  }

  if(segments[digit][5]) {
    digits[1][pos] = '|';
  }

  if(segments[digit][6]) {
    digits[1][pos+1] = '_';
  }
}

void print_digits_array(void) {
  int i, j;

  for(i = 0; i < 3; i++) {
    for(j = 0; j < MAX_DIGITS*4; j++)
      printf("%c",digits[i][j]);
    printf("\n");
  }
}
