#include <stdio.h>

void test1(const int a);
void test2(const int* a);
void test3(const int* const a);

int main(int argc, char *argv[])
{

  int  i = 10;
  test1(i);
  test2(&i);
  test3(&i);
  return 0;
}


void test1(const int a) {
  a = 3;
}
void test2(const int* a) {
  //*a = 3;
  int* p;
  a = p;
}
void test3(const int* const a) {
  //*a = 3;
  int* p;
  a = p;
}
