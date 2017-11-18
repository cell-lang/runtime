#include "lib.h"
#include "os-interface.h"


uint64 get_tick_count() {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    // error
  }
  return 1000 * ts.tv_sec + ts.tv_nsec / 1000000;
}

char *file_read(const char *fname, int &size) {
  FILE *fp = fopen(fname, "r");
  if (fp == NULL) {
    size = -1;
    return NULL;
  }
  int start = ftell(fp);
  assert(start == 0);
  fseek(fp, 0, SEEK_END);
  int end = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  size = end - start;
  if (size == 0) {
    fclose(fp);
    return NULL;
  }
  char *buff = new_byte_array(size);
  int read = fread(buff, 1, size, fp);
  fclose(fp);
  if (read != size) {
    delete_byte_array(buff, size);
    size = -1;
    return NULL;
  }
  return buff;
}

bool file_write(const char *fname, const char *buffer, int size, bool append) {
  FILE *fp = fopen(fname, append ? "a" : "w");
  size_t written = fwrite(buffer, 1, size, fp);
  fclose(fp);
  return written == size;
}
