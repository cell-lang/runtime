#include "lib.h"
#include "os-interface.h"


namespace generated {
  struct ENV;
}


OBJ FileRead_P(OBJ filename, generated::ENV &) {
  char *fname = obj_to_str(filename);
  int size;
  char *data = file_read(fname, size);
  delete_byte_array(fname, strlen(fname)+1);

  if (size == -1)
    return make_symb(symb_idx_nothing);

  OBJ seq_obj = make_empty_seq();
  if (size > 0) {
    SEQ_OBJ *seq = new_seq(size);
    for (uint32 i=0 ; i < size ; i++)
      seq->buffer[i] = make_int((uint8) data[i]);
    delete_byte_array(data, size);
    seq_obj = make_seq(seq, size);
  }

  return make_tag_obj(symb_idx_just, seq_obj);
}


OBJ FileWrite_P(OBJ filename, OBJ data, generated::ENV &) {
  char *fname = obj_to_str(filename);
  uint32 size;
  char *buffer = obj_to_byte_array(data, size);
  bool res;
  if (size > 0) {
    res = file_write(fname, buffer, size, false);
    delete_byte_array(buffer, size);
  }
  else {
    char empty_buff[1];
    res = file_write(fname, empty_buff, 0, false);
  }
  delete_byte_array(fname, strlen(fname)+1);
  return make_bool(res);
}


OBJ Print_P(OBJ str_obj, generated::ENV &env) {
  char *str = obj_to_str(str_obj);
  fputs(str, stdout);
  fflush(stdout);
  delete_byte_array(str, strlen(str)+1);
  return make_blank_obj();
}


OBJ GetChar_P(generated::ENV &env) {
  int ch = getchar();
  if (ch == EOF)
    return make_symb(symb_idx_nothing);
  return make_tag_obj(symb_idx_just, make_int(ch));
}
