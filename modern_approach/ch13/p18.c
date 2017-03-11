#include <stdio.h>

char *s[12] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November",
  "December"
};

int main(int argc, char *argv[])
{
  int m, d, y;
  printf("Enter a date(mm/dd/yyyy):");
  scanf("%d/%d/%d", &m, &d, &y);
  printf("You entered the date %s %d, %d",
         s[m-1], d, y);
  return 0;
}
