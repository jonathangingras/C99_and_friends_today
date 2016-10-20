#include <dlfcn.h>
#include <stdarg.h>
#include <stdlib.h>

int (*printf_orig)(const char *fmt, ...);

__attribute__((constructor)) static void at_load_time() {
  printf_orig = dlsym(RTLD_NEXT, "printf");
}

int printf(const char *fmt, ...) {
  printf_orig("no way !\n");

  return 10;
}

void *fopen(const char *filename, const char *mode) {
  printf_orig("ah ah tranna open file\n");
  return NULL;
}

int access(const char *name, int mode) {
  printf_orig("I will always return -1\n");
  return -1;
}

int puts(const char *str) {
  printf_orig("no way either!\n");

  return 16;
}
