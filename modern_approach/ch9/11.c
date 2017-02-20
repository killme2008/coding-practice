#include <stdio.h>
#include <ctype.h>
#define N 5

float compute_GPA(char grades[], int n);

int main(int argc, char *argv[])
{
  char grades[N];
  printf("Enter grades: ");
  for(int i = 0; i < N; i++)
    scanf(" %c", &grades[i]);

  for(int i = 0; i < N; i++)
    printf(" %c", grades[i]);
  printf("\n");

  printf("GPA: %.2f", compute_GPA(grades, N));

  return 0;
}
float compute_GPA(char grades[], int n) {
  float sum = 0;
  for(int i = 0; i < n; i++){
    switch(toupper(grades[i])) {
    case 'A':
      sum += 4.0f;
      break;
    case 'B':
      sum += 3.0f;
      break;
    case 'C':
      sum += 2.0f;
      break;
    case 'D':
      sum += 1.0f;
      break;
    case 'F':
    default:
      break;
    }
  }
  return sum;
}
