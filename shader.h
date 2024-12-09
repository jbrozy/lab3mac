#include <stdio.h>
#include <stdlib.h>

char *read_from_file(const char *path) {
  char *buffer = 0;
  long length;
  FILE *f = fopen(path, "rb");

  if (f) {
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (char *)malloc(length);
    if (buffer) {
      fread(buffer, 1, length, f);
    }
    fclose(f);
  }

  return buffer;
}
