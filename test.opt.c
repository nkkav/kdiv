// test.opt.c
#include <stdio.h>
#include <stdlib.h>
inline signed int kdiv_s32_p_23 (signed int n)
{
  signed int q, M=-1307163959, c;
  signed long long int t, u, v;
  t = (signed long long int)M * (signed long long int)n;
  q = t >> 32;
  q = q + n;
  q = q >> 4;
  c = n >> 31;
  q = q + c;
  return (q);
}

int main(int argc, char *argv[]) {
  int a, b;
  a = atoi(argv[1]);
  b = kdiv_s32_p_23(a);
  printf("b = %d\n", b);
  return b;
}
