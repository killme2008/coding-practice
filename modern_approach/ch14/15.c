#include <stdio.h>

/**
   gcc -D ENGLISH=1 15.c
 */

int main(int argc, char *argv[])
{
#if defined(ENGLISH)
  puts("Insert Disk 1");
#elif defined(FRENCH)
  puts("Inserez Le Disque 1");
#elif defined(SPANISH)
  puts("Insertz El Disco 1");
#endif
  return 0;
}
