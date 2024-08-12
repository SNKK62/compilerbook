#include <stdio.h>

int foo() {
  printf("foo has called!!!! foooooo!!!\n");
  return 1;
}

int bar(int a, int b, int c) {
  printf("bar has called!!!! with a:%d, b:%d, c:%d\n", a, b, c);
  return 1;
}

