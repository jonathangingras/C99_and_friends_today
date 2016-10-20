#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  char str[] = "hello its me\n";
  printf("hello");

  printf("How you doin %s?", "girl");

  printf(str);

  printf("will not be overwritten\n");

  return 0;
}
