#include "lib.h"


std::vector<const char *> function_names;
std::vector<uint32>       arities;
std::vector<OBJ *>        param_lists;

void push_call_info(const char *fn_name, uint32 arity, OBJ *params) {
#ifndef NDEBUG
  function_names.push_back(fn_name);
  arities.push_back(arity);
  param_lists.push_back(params);
#endif
}

void pop_call_info() {
#ifndef NDEBUG
  uint32 arity = arities.back();
  if (arity > 0)
    delete_obj_array(param_lists.back(), arity);

  function_names.pop_back();
  arities.pop_back();
  param_lists.pop_back();
#endif
}

void pop_try_mode_call_info(int depth) {
  while (function_names.size() > depth)
    pop_call_info();
}

int get_call_stack_depth() {
  return function_names.size();
}

////////////////////////////////////////////////////////////////////////////////

void printed_obj_or_filename(OBJ obj, bool add_path, char *buffer, uint32 buff_size) {
  const uint32 MAX_OBJ_COUNT = 1024;
  static uint32 filed_objs_count = 0;
  // Deliberate bug: storing objects without reference counting them.
  static OBJ filed_objs[MAX_OBJ_COUNT];

  assert(buff_size >= 64);

  const char *file_template = add_path ? "<debug/obj_%02d.txt>" : "<obj_%02d.txt>";

  for (uint32 i=0 ; i < filed_objs_count ; i++)
    if (are_eq(filed_objs[i], obj)) {
      sprintf(buffer, file_template, i);
      return;
    }

  char fname[64];
  sprintf(fname, "debug/obj_%02d.txt", filed_objs_count);

  print_to_buffer_or_file(obj, buffer, buff_size, fname);

  if (buffer[0] == '\0') {
    // The object was written to a file
    sprintf(buffer, file_template, filed_objs_count);
    if (filed_objs_count < MAX_OBJ_COUNT)
      filed_objs[filed_objs_count++] = obj;
  }
}

////////////////////////////////////////////////////////////////////////////////

void print_indented_param(FILE *fp, OBJ param, bool is_last) {
  const uint32 BUFF_SIZE = 512;
  char buffer[BUFF_SIZE];

  if (!is_blank_obj(param))
    printed_obj_or_filename(param, false, buffer, BUFF_SIZE);
  else
    strcpy(buffer, "<closure>");

  for (uint32 i=0 ; buffer[i] != '\0' ; i++) {
    if (i == 0 || buffer[i-1] == '\n')
      fputs("  ", fp);
    fputc(buffer[i], fp);
  }

  if (!is_last)
    fputs(",", fp);
  fputs("\n", fp);
  fflush(fp);
}


void print_stack_frame(FILE *fp, uint32 frame_idx) {
  const char *fn_name = function_names[frame_idx];
  uint32 arity = arities[frame_idx];
  OBJ *params = param_lists[frame_idx];

  fputs(fn_name, fp);
  fputs("(", fp);
  if (arity > 0)
    fputs("\n", fp);
  for (uint32 i=0 ; i < arity ; i++)
    print_indented_param(fp, params[i], i == arity-1);
  fputs(")\n\n", fp);
}


void print_stack_frame(uint32 frame_idx) {
  const char *fn_name = function_names[frame_idx];
  fprintf(stderr, "%s\n", fn_name);
}


void print_call_stack() {
#ifndef NDEBUG
  uint32 size = function_names.size();
  for (uint32 i=0 ; i < size ; i++)
    print_stack_frame(i);
  fputs("\nNow trying to write a full dump of the stack to the file debug/stack_trace.txt.\nPlease be patient. This may take a while...", stderr);
  fflush(stderr);
  FILE *fp = fopen("debug/stack_trace.txt", "w");
  if (fp == NULL) {
    fputs("\nFailed to open file debug/stack_trace.txt\n", stderr);
    return;
  }
  for (uint32 i=0 ; i < size ; i++)
    print_stack_frame(fp, i);
  fputs(" done.\n\n", stderr);
  fclose(fp);
#endif
}


void dump_var(const char *name, OBJ value) {
  const uint32 BUFF_SIZE = 512;
  char buffer[BUFF_SIZE];
  printed_obj_or_filename(value, true, buffer, BUFF_SIZE);
  fprintf(stderr, "%s = %s\n\n", name, buffer);
}

////////////////////////////////////////////////////////////////////////////////

void print_assertion_failed_msg(const char *file, uint32 line, const char *text) {
  if (text == NULL)
    fprintf(stderr, "\nAssertion failed. File: %s, line: %d\n\n", file, line);
  else
    fprintf(stderr, "\nAssertion failed: %s\nFile: %s, line: %d\n\n", text, file, line);
}

////////////////////////////////////////////////////////////////////////////////

void soft_fail(const char *msg) {
#ifndef CELL_LANG_NO_TRANSACTIONS
  if (is_in_try_state())
    throw 0LL;
#endif

  if (msg != NULL)
    fprintf(stderr, "%s\n\n", msg);
  print_call_stack();
  *(char *)0 = 0;
}

void impl_fail(const char *msg) {
  if (msg != NULL)
    fprintf(stderr, "%s\n\n", msg);
  print_call_stack();
  *(char *)0 = 0;
}

void internal_fail() {
  fputs("Internal error!\n", stderr);
  fflush(stderr);
  print_call_stack();
  *(char *)0 = 0;
}
