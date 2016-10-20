#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

int CompileCCode(const char *filename) {
  if(access(filename, F_OK)) {
    printf("no such filename\n");
  } else {
    char cmd_1[] = "cc -w -shared -fPIC -o ";
    char cmd[strlen(cmd_1) + 2*strlen(filename) + 5];
    cmd[0] = 0;
    strcat(cmd, cmd_1);
    strcat(cmd, filename);
    strcat(cmd, ".so ");
    strcat(cmd, filename);

    printf("command: %s\n", cmd);

    system(cmd);

    char libfilename[strlen(filename) + 4];
    libfilename[0] = 0;
    strcat(libfilename, filename);
    strcat(libfilename, ".so");

    printf("libfilename: %s\n", libfilename);

    if(access(filename, F_OK) == 0) {
      void *handle = dlopen(libfilename, RTLD_LAZY);

      if(!handle) {
        printf("could not load lib\n");
        return -1;
      }

      int (*libmain)() = dlsym(handle, "libmain");

      if(libmain) {
        libmain();
      } else {
        return -1;
      }
    } else {
      printf("lib file not found\n");
      return -1;
    }
  }

  return 0;
}

int main(int argc, char **argv) {
  if(argc == 2) CompileCCode(argv[1]);

  return 0;
}
