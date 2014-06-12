// test.c
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
  int a, b;
  a = atoi(argv[1]);
  b = a / 23;
  printf("b = %d\n", b);
  return b;
}
