#include "lib.h"


bool _assert_(int exp, const char *exp_text, const char *file, int line) {
  if (!exp) {
    int idx = 0;
    while (file[idx] != '\0')
      idx++;

    while (idx >= 0 && file[idx] != '\\')
      idx--;

    fprintf(stderr, "Assertion \"%s\" failed, file: %s, line: %d\n", exp_text, file + idx + 1, line);
    fflush(stderr);

    (*((char *)0)) = 0;
  }

  return true;
}


void mantissa_and_dec_exp(double value, long long &mantissa, int &dec_exp) {
  char buffer[1024];
  sprintf(buffer, "%f", value);

  int len = strlen(buffer);
  char *dot_ptr = strchr(buffer, '.');
  int dot_idx = dot_ptr - buffer;
  dec_exp = 0;
  if (dot_ptr == NULL) {
    for (int i=len-1 ; i >= 0  && buffer[i] == '\0' ; i++)
      dec_exp++;
    len -= dec_exp;
    buffer[len] = '\0';
  }
  else {
    memmove(dot_ptr, dot_ptr+1, len-dot_idx);
    dec_exp = dot_idx - len + 1;
    len--;
  }

  sscanf(buffer, "%lld", &mantissa);
}
