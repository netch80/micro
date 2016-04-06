#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <err.h>
#include "softfloat_types.h"
#include "softfloat.h"

#define ITERS 10000000

volatile double r;

long tsdiff(struct timespec* ts1, struct timespec* ts0) {
  return (ts1->tv_sec - ts0->tv_sec) * 1000000000l +
      (ts1->tv_nsec - ts0->tv_nsec);
}

int main(int argc, char *argv[]) {
  struct timespec ts0, ts1;
  double a, b;
  int i;
  if (argc < 3) {
    errx(1, "usage");
  }
  a = strtod(argv[1], NULL);
  b = strtod(argv[2], NULL);
  clock_gettime(CLOCK_MONOTONIC, &ts0);
  for (i = 0; i < ITERS; ++i) {
    r = a + b;
  }
  clock_gettime(CLOCK_MONOTONIC, &ts1);
  printf("hard -> r=%30.25g t=%20.9f\n",
      r, tsdiff(&ts1, &ts0) / (double) ITERS);
  clock_gettime(CLOCK_MONOTONIC, &ts0);
  for (i = 0; i < ITERS; ++i) {
    float64_t xa, xb, xr;
#if MEMCOPY
    memmove(&xa, &a, 8);
    memmove(&xb, &b, 8);
#else
    xa = *(float64_t*)&a;
    xb = *(float64_t*)&b;
#endif
    xr = f64_add(xa, xb);
#if MEMCOPY
    memmove((void*)&r, &xr, 8);
#else
    r = *(double*)&xr;
#endif
  }
  clock_gettime(CLOCK_MONOTONIC, &ts1);
  printf("soft -> r=%30.25g t=%20.9f\n",
      r, tsdiff(&ts1, &ts0) / (double) ITERS);
  return 0;
}
// vim: set ts=2 sts=2 sw=2 et :
