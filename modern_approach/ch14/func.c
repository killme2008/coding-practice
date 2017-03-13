#include <stdio.h>

#define FUNC_CALLED() printf("%s called\n", __func__);
#define FUNC_RETURN() printf("%s returns\n", __func__);

void a() {
  FUNC_CALLED();
  FUNC_RETURN();
}

void b() {
  FUNC_CALLED();
  a();
  FUNC_RETURN();
}

void c() {
  FUNC_CALLED();
  b();
  FUNC_RETURN();
}


int main(int argc, char *argv[])
{
  /**
     c called
     b called
     a called
     a returns
     b returns
     c returns
   */
  c();
  return 0;
}
