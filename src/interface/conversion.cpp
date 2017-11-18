#include "lib.h"
#include <string>

using std::string;


OBJ convert_bool_seq(const bool *array, uint32 size) {
  if (size == 0)
    return make_empty_seq();
  //## CHECK THAT THE INPUT ARRAY DOES NOT EXCEED THE MAXIMUM SEQUENCE SIZE
  SEQ_OBJ *seq = new_seq(size);
  OBJ *buffer = seq->buffer;
  for (uint32 i=0 ; i < size ; i++)
    buffer[i] = make_bool(array[i]);
  return make_seq(seq, size);
}

OBJ convert_int32_seq(const int32 *array, uint32 size) {
  if (size == 0)
    return make_empty_seq();
  //## CHECK THAT THE INPUT ARRAY DOES NOT EXCEED THE MAXIMUM SEQUENCE SIZE
  SEQ_OBJ *seq = new_seq(size);
  OBJ *buffer = seq->buffer;
  for (unsigned int i=0 ; i < size ; i++)
    buffer[i] = make_int(array[i]);
  return make_seq(seq, size);
}

OBJ convert_int_seq(const int64 *array, uint32 size) {
  if (size == 0)
    return make_empty_seq();
  //## CHECK THAT THE INPUT ARRAY DOES NOT EXCEED THE MAXIMUM SEQUENCE SIZE
  SEQ_OBJ *seq = new_seq(size);
  OBJ *buffer = seq->buffer;
  for (unsigned int i=0 ; i < size ; i++)
    buffer[i] = make_int(array[i]);
  return make_seq(seq, size);
}

OBJ convert_float_seq(const double *array, uint32 size) {
  if (size == 0)
    return make_empty_seq();
  //## CHECK THAT THE INPUT ARRAY DOES NOT EXCEED THE MAXIMUM SEQUENCE SIZE
  SEQ_OBJ *seq = new_seq(size);
  OBJ *buffer = seq->buffer;
  for (uint32 i=0 ; i < size ; i++)
    buffer[i] = make_float(array[i]);
  return make_seq(seq, size);
}

OBJ convert_text(const char *buffer) {
  OBJ obj;
  uint32 error_offset;
  bool ok = parse(buffer, strlen(buffer), &obj, &error_offset);
  if (!ok)
    throw (long long) error_offset;
  return obj;
}

////////////////////////////////////////////////////////////////////////////////

// void export_as_c_string(OBJ obj, char *buffer, uint32 capacity) {
//   obj_to_str(obj, buffer, capacity);
// }

uint32 export_as_bool_array(OBJ obj, bool *array, uint32 capacity) {
  uint32 len = get_seq_length(obj);
  if (len >= capacity)
    throw (long long) len;
  OBJ *buffer = get_seq_buffer_ptr(obj);
  for (uint32 i=0 ; i < len ; i++)
    array[i] = get_bool(buffer[i]);
  return len;
}

uint32 export_as_long_long_array(OBJ obj, int64 *array, uint32 capacity) {
  uint32 len = get_seq_length(obj);
  if (len >= capacity)
    throw (long long) len;
  OBJ *buffer = get_seq_buffer_ptr(obj);
  for (uint32 i=0 ; i < len ; i++)
    array[i] = get_int(buffer[i]);
  return len;
}

uint32 export_as_float_array(OBJ obj, double *array, uint32 capacity) {
  uint32 len = get_seq_length(obj);
  if (len >= capacity)
    throw (long long) len;
  OBJ *buffer = get_seq_buffer_ptr(obj);
  for (uint32 i=0 ; i < len ; i++)
    array[i] = get_float(buffer[i]);
  return len;
}

void export_literal_as_c_string(OBJ obj, char *buffer, uint32 capacity) {
  uint32 min_capacity = printed_obj(obj, buffer, capacity);
  if (capacity < min_capacity)
    throw (long long) min_capacity;
}

////////////////////////////////////////////////////////////////////////////////

string export_as_std_string(OBJ obj) {
  string result;
  OBJ raw_str_obj = get_inner_obj(obj);
  if (!is_empty_seq(raw_str_obj)) {
    OBJ *seq_buffer = get_seq_buffer_ptr(raw_str_obj);
    uint32 len = get_seq_length(raw_str_obj);
    result.resize(len);
    for (uint32 i=0 ; i < len ; i++)
      result[i] = get_int_val(seq_buffer[i]);
  }
  return result;
}

// #include <sstream>
//
// using std::ostringstream;
//
// void emit_stream(void *ptr, const void *data, EMIT_ACTION action) {
//   ostringstream &stream = *(ostringstream *) ptr;
//   if (action == TEXT)
//     stream << (char *) data;
// }
//
// string export_literal_as_std_string(OBJ obj) {
//   ostringstream stream;
//   print_obj(obj, emit_stream, &stream);
//   return stream.str();
// }
