#include<stdio.h>

int main(int argc, char *argv[])
{
  int hour,min;
  char* label;

  printf("Enter a 24-hour time: ");
  scanf("%d:%d", &hour, &min);


  label = hour<12 ? "AM" : "PM";
  if(hour > 12)
    hour %= 12;

  printf("Equivalment 12-hour time: %d:%02d %s", hour, min, label);

  return 0;
}
