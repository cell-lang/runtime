#include "lib.h"


bool is_str(uint16 tag_idx, OBJ obj) {
  if (tag_idx != symb_idx_string)
    return false;

  if (is_empty_seq(obj))
    return true;

  if (!is_ne_seq(obj))
    return false;

  uint32 len = get_seq_length(obj);
  OBJ *elems = get_seq_buffer_ptr(obj);

  for (uint32 i=0 ; i < len ; i++) {
    OBJ elem = elems[i];

    if (!is_int(elem))
      return false;

    int64 value = get_int_val(elem);
    if (value < 0 | value >= 65536)
      return false;
  }

  return true;
}


bool is_record(OBJ obj) {
  if (!is_ne_map(obj))
    return false;

  BIN_REL_OBJ *map = get_bin_rel_ptr(obj);
  uint32 size = map->size;
  OBJ *keys = get_left_col_array_ptr(map);

  for (uint32 i=0 ; i < size ; i++)
    if (!is_symb(keys[i]))
      return false;

  return true;
}


void print_bare_str(OBJ str, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  char buffer[64];

  assert(is_str(get_tag_idx(str), get_inner_obj(str)));

  OBJ char_seq = get_inner_obj(str);
  if (is_empty_seq(char_seq))
    return;

  uint32 len = get_seq_length(char_seq);
  OBJ *chars = get_seq_buffer_ptr(char_seq);

  for (uint32 i=0 ; i < len ; i++) {
    int64 ch = get_int_val(chars[i]);
    assert(ch >= 0 & ch < 65536);
    if (ch >= ' ' & ch <= '~') {
      buffer[0] = '\\';
      buffer[1] = ch;
      buffer[2] = '\0';
      emit(data, buffer + (ch == '"' | ch == '\\' ? 0 : 1), TEXT);
    }
    else if (ch == '\n') {
      emit(data, "\\n", TEXT);
    }
    else if (ch == '\t') {
      emit(data, "\\t", TEXT);
    }
    else {
      sprintf(buffer, "\\%04llx", ch);
      emit(data, buffer, TEXT);
    }
  }
}


void print_int(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  int64 n = get_int(obj);
  char buffer[1024];
  sprintf(buffer, "%lld", n);
  emit(data, buffer, TEXT);
}


void print_float(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  double x = get_float(obj);
  char buffer[1024];
  sprintf(buffer, "%g", x);
  bool is_integer = true;
  for (int i=0 ; buffer[i] != '\0' ; i++)
    if (!isdigit(buffer[i])) {
      is_integer = false;
      break;
    }
  if (is_integer)
    strcat(buffer, ".0");
  emit(data, buffer, TEXT);
}


void print_symb(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  OBJ str = to_str(obj);
  print_bare_str(str, emit, data);
  release(str);
}


void print_seq(OBJ obj, bool print_parentheses, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  if (print_parentheses)
    emit(data, "(", TEXT);
  if (!is_empty_seq(obj)) {
    uint32 len = get_seq_length(obj);
    OBJ *elems = get_seq_buffer_ptr(obj);
    for (uint32 i=0 ; i < len ; i++) {
      if (i > 0)
        emit(data, ", ", TEXT);
      print_obj(elems[i], emit, data);
    }
  }
  if (print_parentheses)
    emit(data, ")", TEXT);
}


void print_set(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  emit(data, "[", TEXT);
  if (!is_empty_rel(obj)) {
    SET_OBJ *set = get_set_ptr(obj);
    uint32 size = set->size;
    OBJ *elems = set->buffer;
    for (uint32 i=0 ; i < size ; i++) {
      if (i > 0)
        emit(data, ", ", TEXT);
      print_obj(elems[i], emit, data);
    }
  }
  emit(data, "]", TEXT);
}


void print_ne_bin_rel(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  emit(data, "[", TEXT);

  BIN_REL_OBJ *rel = get_bin_rel_ptr(obj);
  uint32 size = rel->size;
  OBJ *left_col = get_left_col_array_ptr(rel);
  OBJ *right_col = get_right_col_array_ptr(rel);

  for (uint32 i=0 ; i < size ; i++) {
    if (i > 0)
      emit(data, "; ", TEXT);
    emit(data, NULL, SUB_START);
    print_obj(left_col[i], emit, data);
    emit(data, ", ", TEXT);
    print_obj(right_col[i], emit, data);
    emit(data, NULL, SUB_END);
  }

  if (size == 1)
    emit(data, ";", TEXT);

  emit(data, "]", TEXT);
}


void print_ne_map(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  BIN_REL_OBJ *map = get_bin_rel_ptr(obj);
  uint32 size = map->size;
  OBJ *keys = get_left_col_array_ptr(map);
  OBJ *values = get_right_col_array_ptr(map);

  emit(data, "[", TEXT);

  for (uint32 i=0 ; i < size ; i++) {
    if (i > 0)
      emit(data, ", ", TEXT);
    emit(data, NULL, SUB_START);
    print_obj(keys[i], emit, data);
    emit(data, " -> ", TEXT);
    print_obj(values[i], emit, data);
    emit(data, NULL, SUB_END);
  }

  emit(data, "]", TEXT);
}


void print_record(OBJ obj, bool print_parentheses, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  if (print_parentheses)
    emit(data, "(", TEXT);

  BIN_REL_OBJ *map = get_bin_rel_ptr(obj);
  uint32 size = map->size;
  OBJ *keys = get_left_col_array_ptr(map);
  OBJ *values = get_right_col_array_ptr(map);

  for (uint32 i=0 ; i < size ; i++) {
    if (i > 0)
      emit(data, ", ", TEXT);
    emit(data, NULL, SUB_START);
    print_symb(keys[i], emit, data);
    emit(data, ": ", TEXT);
    print_obj(values[i], emit, data);
    emit(data, NULL, SUB_END);
  }

  if (print_parentheses)
    emit(data, ")", TEXT);
}


void print_ne_tern_rel(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  emit(data, "[", TEXT);

  TERN_REL_OBJ *rel = get_tern_rel_ptr(obj);
  uint32 size = rel->size;
  OBJ *col1 = get_col_array_ptr(rel, 0);
  OBJ *col2 = get_col_array_ptr(rel, 1);
  OBJ *col3 = get_col_array_ptr(rel, 2);

  for (uint32 i=0 ; i < size ; i++) {
    if (i > 0)
      emit(data, "; ", TEXT);
    emit(data, NULL, SUB_START);
    print_obj(col1[i], emit, data);
    emit(data, ", ", TEXT);
    print_obj(col2[i], emit, data);
    emit(data, ", ", TEXT);
    print_obj(col3[i], emit, data);
    emit(data, NULL, SUB_END);
  }

  if (size == 1)
    emit(data, ";", TEXT);

  emit(data, "]", TEXT);
}


void print_tag_obj(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  uint16 tag_idx = get_tag_idx(obj);
  OBJ inner_obj = get_inner_obj(obj);
  if (is_str(tag_idx, inner_obj)) {
    emit(data, "\"", TEXT);
    print_bare_str(obj, emit, data);
    emit(data, "\"", TEXT);
  }
  else {
    print_symb(make_symb(tag_idx), emit, data);
    emit(data, "(", TEXT);

    if (is_record(inner_obj))
      print_record(inner_obj, false, emit, data);
    else if (is_ne_seq(inner_obj) && get_seq_length(inner_obj) > 1)
      print_seq(inner_obj, false, emit, data);
    else
      print_obj(inner_obj, emit, data);

    emit(data, ")", TEXT);
  }
}


void print_obj(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data) {
  emit(data, NULL, SUB_START);

  if (is_blank_obj(obj))
    emit(data, "BLANK", TEXT);

  else if (is_null_obj(obj))
    emit(data, "NULL", TEXT);

  else if (is_int(obj))
    print_int(obj, emit, data);

  else if (is_float(obj))
    print_float(obj, emit, data);

  else if (is_symb(obj))
    print_symb(obj, emit, data);

  else if (is_seq(obj))
    print_seq(obj, true, emit, data);

  else if (is_set(obj))
    print_set(obj, emit, data);

  else if (is_record(obj))
    print_record(obj, true, emit, data);

  else if (is_ne_map(obj)) //## SHOULD I PRINT IT AS A MAP ONLY WHEN IT'S A PHYSICAL ONE?
    print_ne_map(obj, emit, data);

  else if (is_ne_bin_rel(obj))
    print_ne_bin_rel(obj, emit, data);

  else if (is_ne_tern_rel(obj))
    print_ne_tern_rel(obj, emit, data);

  else // is_tag_obj(obj)
    print_tag_obj(obj, emit, data);

  emit(data, NULL, SUB_END);
}


struct TEXT_FRAG {
  int depth;
  uint32 start;
  uint32 length;
};


struct PRINT_BUFFER {
  uint32 str_len;
  char *buffer;
  uint32 buff_size;

  uint32 frags_count;
  TEXT_FRAG *fragments;
  uint32 frags_buff_size;

  int curr_depth;
};


void init(PRINT_BUFFER *pb) {
  pb->str_len = 0;
  pb->buffer = new_byte_array(4096);
  pb->buff_size = 4096;
  pb->buffer[0] = '\0';

  pb->frags_count = 0;
  pb->fragments = (TEXT_FRAG *) new_void_array(4096);
  pb->frags_buff_size = 4096;

  pb->curr_depth = -1;
}


void cleanup(PRINT_BUFFER *pb) {
  delete_byte_array(pb->buffer, pb->buff_size);
  delete_void_array(pb->fragments, pb->frags_buff_size);
}


void adjust_buff_capacity(PRINT_BUFFER *pb, uint32 extra_capacity) {
  uint32 buff_size = pb->buff_size;
  uint32 min_capacity = pb->str_len + extra_capacity + 1;
  if (buff_size < min_capacity) {
    uint32 new_capacity = 2 * buff_size;
    while (new_capacity < min_capacity)
      new_capacity *= 2;
    char *new_buff = new_byte_array(new_capacity);
    memcpy(new_buff, pb->buffer, pb->str_len+1);
    delete_byte_array(pb->buffer, buff_size);
    pb->buffer = new_buff;
    pb->buff_size = new_capacity;
  }
}


TEXT_FRAG *insert_new_fragment(PRINT_BUFFER *pb) {
  uint32 curr_capacity = pb->frags_buff_size;
  uint32 frags_count = pb->frags_count;
  uint32 min_capacity = sizeof(TEXT_FRAG) * (frags_count + 1);
  if (curr_capacity < min_capacity) {
    TEXT_FRAG *new_frags = (TEXT_FRAG *) new_void_array(2 * curr_capacity);
    memcpy(new_frags, pb->fragments, sizeof(TEXT_FRAG) * frags_count);
    delete_void_array(pb->fragments, curr_capacity);
    pb->fragments = new_frags;
    pb->frags_buff_size = 2 * curr_capacity;
  }
  pb->frags_count = frags_count + 1;
  return pb->fragments + frags_count;
}


uint32 printable_frags_count(PRINT_BUFFER *pb) {
  uint32 fc = pb->frags_count;
  TEXT_FRAG *fs = pb->fragments;

  TEXT_FRAG *lf = fs + fc - 1;
  assert(pb->curr_depth == lf->depth);

  if (lf->depth == -1)
    return fc - 1;

  uint32 curr_length = lf->length;

  for (uint32 i=fc-2 ; i >= 0 ; i--) {
    TEXT_FRAG *f = fs + i;
    curr_length += f->length;
    if (curr_length > 100)
      return i + 1;
  }

  return 0;
}


void calculate_subobjects_lengths(PRINT_BUFFER *pb, int32 *ls) {
  int fc = pb->frags_count;
  TEXT_FRAG *fs = pb->fragments;

  for (int i=0 ; i < fc ; i++) {
    TEXT_FRAG *f = fs + i;

    int pd;
    if (i == 0)
      pd = -1;
    else
      pd = fs[i-1].depth;

    if (f->depth > pd) {
      int len = 0;
      for (int j=i ; j < fc ; j++) {
        TEXT_FRAG *f2 = fs + j;
        if (f2->depth < f->depth)
          break;
        len += f2->length;
      }
      ls[i] = len;
    }
    else
      ls[i] = -1;
  }
}


void emit_known(PRINT_BUFFER *pb, void (*emit)(void *, const char *, uint32), void *data) {
  int pfc = printable_frags_count(pb);

  int32 *ls = new_int32_array(pb->frags_count);
  calculate_subobjects_lengths(pb, ls);

  char *buff = pb->buffer;
  TEXT_FRAG *fs = pb->fragments;

  int split_depth = ls[0] > 100 ? 0 : -1;

  for (int i=0 ; i < pfc ; i++) {
    TEXT_FRAG *f = fs + i;
    TEXT_FRAG *nf = f + 1;

    int len = f->length;

    int d = f->depth;
    int nd = nf->depth;

    assert(d == nd - 1 || d == nd + 1);
    assert(split_depth <= d);

    if (nd > d) {
      emit(data, buff + f->start, len);

      if (d <= split_depth) {
        if (len >= 2) {
          emit(data, "\n", 1);
          for (int j=0 ; j < nd ; j++)
            emit(data, "  ", 2);
        }
        else if (len == 1) {
          emit(data, " ", 1);
        }
      }

      if (ls[i+1] > 100) {
        assert(split_depth == d);
        split_depth = nd;
      }
    }
    else {
      assert(nd < d);
      if (nd < split_depth) {
        assert(split_depth == d);
        split_depth = nd;
        if (len > 0) {
          emit(data, "\n", 1);
          for (int j=0 ; j <= nd ; j++)
            emit(data, "  ", 2);
        }
      }
      emit(data, buff + f->start, len);
    }
  }
  delete_int32_array(ls, pb->frags_count);
}


void process_text(PRINT_BUFFER *pb, const char *text) {
  int len = strlen(text);
  adjust_buff_capacity(pb, len);
  memcpy(pb->buffer + pb->str_len, text, len+1);
  pb->str_len += len;
  TEXT_FRAG *curr_frag = pb->fragments + pb->frags_count - 1;
  assert(curr_frag->depth == pb->curr_depth);
  curr_frag->length += len;
}


void subobj_start(PRINT_BUFFER *pb) {
  int new_depth = pb->curr_depth + 1;
  pb->curr_depth = new_depth;

  TEXT_FRAG *new_frag = insert_new_fragment(pb);
  new_frag->depth = new_depth;
  new_frag->start = pb->str_len;
  new_frag->length = 0;
}


void subobj_end(PRINT_BUFFER *pb) {
  int new_depth = pb->curr_depth - 1;
  pb->curr_depth = new_depth;

  TEXT_FRAG *new_frag = insert_new_fragment(pb);
  new_frag->depth = new_depth;
  new_frag->start = pb->str_len;
  new_frag->length = 0;
}


////////////////////////////////////////////////////////////////////////////////

void emit_store(void *pb_, const void *data, EMIT_ACTION action) {
  PRINT_BUFFER *pb = (PRINT_BUFFER *) pb_;

  switch (action) {
    case TEXT:
      process_text(pb, (char *) data);
      break;
    case SUB_START:
      subobj_start(pb);
      break;
    case SUB_END:
      subobj_end(pb);
      break;
  }
}


void stdout_print(void *, const char *text, uint32 len) {
  fwrite(text, 1, len, stdout);
  fflush(stdout);
}


void print(OBJ obj) {
  PRINT_BUFFER pb;

  init(&pb);
  print_obj(obj, emit_store, &pb);
  fputs("\n", stdout);
  emit_known(&pb, stdout_print, NULL);
  fputs("\n", stdout);
  cleanup(&pb);
}


void write_to_file(void *fp, const char *text, uint32 len) {
  fwrite(text, 1, len, (FILE *) fp);
}


void append_to_string(void *ptr, const char *text, uint32 len) {
  char *str = (char *) ptr;
  int curr_len = strlen(str);
  memcpy(str + curr_len, text, len);
  str[curr_len + len] = '\0';
}


void calc_length(void *ptr, const char *text, uint32 len) {
  uint32 *total_len = (uint32 *) ptr;
  *total_len += len;
}


void print_to_buffer_or_file(OBJ obj, char *buffer, uint32 max_size, const char *fname) {
  PRINT_BUFFER pb;

  init(&pb);
  print_obj(obj, emit_store, &pb);

  uint32 len = 0;
  emit_known(&pb, calc_length, &len);

  buffer[0] = '\0';
  if (len < max_size) {
    emit_known(&pb, append_to_string, buffer);
  }
  else {
    FILE *fp = fopen(fname, "w");
    emit_known(&pb, write_to_file, fp);
    fclose(fp);
  }

  cleanup(&pb);
}


uint32 printed_obj(OBJ obj, char *buffer, uint32 max_size) {
  PRINT_BUFFER pb;

  init(&pb);
  print_obj(obj, emit_store, &pb);

  uint32 len = 0;
  emit_known(&pb, calc_length, &len);

  if (len + 1 < max_size) {
    memcpy(buffer, pb.buffer, len + 1);
  }

  cleanup(&pb);
  return len + 1;
}


char *printed_obj(OBJ obj, char *alloc_buffer(void *, uint32), void *data) {
  PRINT_BUFFER pb;

  init(&pb);
  print_obj(obj, emit_store, &pb);

  uint32 len = 0;
  emit_known(&pb, calc_length, &len);

  char *buffer = alloc_buffer(data, len+1);
  memcpy(buffer, pb.buffer, len + 1);

  cleanup(&pb);
  return buffer;
}
