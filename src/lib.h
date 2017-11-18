#include "utils.h"


typedef signed char       int8;
typedef signed short      int16;
typedef signed int        int32;
typedef signed long long  int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;


enum OBJ_TYPE {
  // Always inline
  TYPE_BLANK_OBJ  = 0,
  TYPE_NULL_OBJ   = 1,
  TYPE_SYMBOL     = 2,
  TYPE_INTEGER    = 3,
  TYPE_FLOAT      = 4,
  // Inline if empty, references otherwise
  TYPE_SEQUENCE   = 5,
  TYPE_SET        = 6,
  TYPE_BIN_REL    = 7,
  TYPE_TERN_REL   = 8,
  // Always references
  TYPE_TAG_OBJ    = 9,
  TYPE_SLICE      = 10,
  TYPE_MAP        = 11,
  TYPE_LOG_MAP    = 12
};

// Heap object can never be of the following types: TYPE_SLICE, TYPE_LOG_MAP
// Never returned by get_logical_type(): TYPE_SLICE, TYPE_MAP, TYPE_LOG_MAP.

const uint32 MAX_INLINE_OBJ_TYPE_VALUE  = TYPE_FLOAT;
const uint32 MAX_OBJ_TYPE_VALUE         = TYPE_SLICE;


enum MEM_LAYOUT {
  INLINE    = 0,
  STD_MEM   = 1,
  TRY_MEM   = 2
};


struct OBJ {
  union {
    int64   int_;
    double  float_;
    void*   ptr;
  } core_data;

  uint64 extra_data;

  // union {
  //   struct {
  //     uint16   symb_idx;
  //     uint16   inner_tag;
  //     uint16   tag;
  //     uint8    unused_byte;
  //     unsigned type        : 4;
  //     unsigned mem_layout  : 2;
  //     unsigned num_tags    : 2;
  //   } std;
  //
  //   struct {
  //     uint32   length;
  //     uint16   tag;
  //     uint8    unused_byte;
  //     unsigned type        : 4;
  //     unsigned mem_layout  : 2;
  //     unsigned num_tags    : 2;
  //   } seq;
  //
  //   struct {
  //     uint32   length;
  //     unsigned offset      : 24;
  //     unsigned type        : 4;
  //     unsigned mem_layout  : 2;
  //     unsigned num_tags    : 2;
  //   } slice;
  //
  //   uint64 word;
  // } extra_data;
};

////////////////////////////////////////////////////////////////////////////////

struct REF_OBJ {
  uint32 ref_count;
};


struct SEQ_OBJ {
  REF_OBJ ref_obj;
  uint32  capacity;
  uint32  size;
  OBJ     buffer[1];
};


struct SET_OBJ {
  REF_OBJ ref_obj;
  uint32  size;
  OBJ     buffer[1];
};


struct BIN_REL_OBJ {
  REF_OBJ ref_obj;
  uint32  size;
  OBJ     buffer[1];
};


struct TERN_REL_OBJ {
  REF_OBJ ref_obj;
  uint32  size;
  OBJ     buffer[1];
};

struct TAG_OBJ {
  REF_OBJ ref_obj;
  uint16 tag_idx;
  uint16 unused_field;
  OBJ    obj;
};

////////////////////////////////////////////////////////////////////////////////

struct SEQ_ITER {
  OBJ    *buffer;
  uint32  idx;
  uint32  len;
};


struct SET_ITER {
  OBJ    *buffer;
  uint32  idx;
  uint32  size;
};


struct BIN_REL_ITER {
  OBJ    *left_col;
  OBJ    *right_col;
  uint32 *rev_idxs; // If this is not null, we are iterating in right column order
  uint32  idx;
  uint32  end; // Non-inclusive upper bound
};


struct TERN_REL_ITER {
  OBJ    *col1;
  OBJ    *col2;
  OBJ    *col3;
  uint32 *ordered_idxs; // If this is null, we iterate directly over the columns
  uint32  idx;
  uint32  end; // Non-inclusive upper bound
};

struct STREAM {
  OBJ    *buffer;
  uint32  capacity;
  uint32  count;
};

////////////////////////////////////////////////////////////////////////////////

struct VALUE_STORE {
  void *ptr;
  uint32 capacity;
  uint32 usage;
  uint32 first_free;
};


struct VALUE_STORE_UPDATES {
  void *ptr;
  uint32 capacity;
  uint32 count;
  uint32 first_free;
};


struct UNARY_TABLE {
  uint64 *bitmap;
  uint32 size;
  uint32 count;
};


struct UNARY_TABLE_UPDATES {
  uint32 capacity;
  uint32 deletes_count;
  uint32 inserts_count;
  uint32 *buffer; // Deletes are stored at the front, inserts at the back
};


struct UNARY_TABLE_ITER {
  uint64 *bitmap;
  uint32 size;
  uint32 curr_value;
};


struct BINARY_TABLE {
  std::set<uint64> left_to_right;
  std::set<uint64> right_to_left;
};


struct BINARY_TABLE_UPDATES {
  std::vector<uint64> deletes;
  std::vector<uint64> inserts;
};


struct BINARY_TABLE_ITER {
  std::set<uint64>::iterator iter;
  std::set<uint64>::iterator end;
  uint32 value;
  bool reversed;
};


struct tuple3 {
  uint64 fields01;
  uint32 field2;

  bool operator == (const tuple3 &o) const {
    return fields01 == o.fields01 & field2 == o.field2;
  }

  bool operator < (const tuple3 &o) const {
    return fields01 != o.fields01 ? fields01 < o.fields01 : field2 < o.field2;
  }
};

// bool operator < (const tuple3 &l, tuple3 &r) {
//   return l.fields01 != r.fields01 ? l.fields01 < r.fields01 : l.field2 < r.field2;
// }


struct TERNARY_TABLE {
  std::set<tuple3> unshifted;
  std::set<tuple3> shifted_once;
  std::set<tuple3> shifted_twice;
};


struct TERNARY_TABLE_UPDATES {
  std::vector<tuple3> deletes;
  std::vector<tuple3> inserts;
};


struct TERNARY_TABLE_ITER {
  std::set<tuple3>::iterator iter;
  std::set<tuple3>::iterator end;
  uint64 excl_upper_bound;
  uint8 shift;
};

///////////////////////////////////////////////////////////////

const uint64 MAX_SEQ_LEN = 0xFFFFFFFF;

const uint32 INVALID_INDEX = 0xFFFFFFFFU;

const uint16 symb_idx_false   = 0;
const uint16 symb_idx_true    = 1;
const uint16 symb_idx_void    = 2;
const uint16 symb_idx_string  = 3;
const uint16 symb_idx_nothing = 4;
const uint16 symb_idx_just    = 5;
const uint16 symb_idx_success = 6;
const uint16 symb_idx_failure = 7;

///////////////////////////////// mem_alloc.cpp ////////////////////////////////

bool is_in_normal_state();
bool is_in_try_state();
bool is_in_copying_state();

void enter_try_state();
void enter_copy_state();
void restore_try_state();
void return_to_normal_state();
void abort_try_state();

//////////////////////////////// mem-copying.cpp ///////////////////////////////

OBJ copy_obj(OBJ obj);

///////////////////////////////// mem-core.cpp /////////////////////////////////

void* new_obj(uint32 byte_size);
void* new_obj(uint32 requested_byte_size, uint32 &returned_byte_size);
void  free_obj(void* obj, uint32 byte_size);
void* resize_obj(void *ptr, uint32 byte_size, uint32 new_byte_size);

bool is_alive(void* obj);

uint32 get_live_objs_count();
uint32 get_max_live_objs_count();
uint32 get_total_objs_count();

uint32 get_live_mem_usage();
uint32 get_max_live_mem_usage();
uint32 get_total_mem_requested();

void print_all_live_objs();

/////////////////////////////////// mem.cpp ////////////////////////////////////

void add_ref(REF_OBJ *);
void add_ref(OBJ);
void release(OBJ);

void vec_add_ref(OBJ* objs, uint32 len);
void vec_release(OBJ* objs, uint32 len);

OBJ* get_left_col_array_ptr(BIN_REL_OBJ*);
OBJ* get_right_col_array_ptr(BIN_REL_OBJ*);
uint32 *get_right_to_left_indexes(BIN_REL_OBJ*);

OBJ *get_col_array_ptr(TERN_REL_OBJ *rel, int idx);
uint32 *get_rotated_index(TERN_REL_OBJ *rel, int amount);

SET_OBJ*      new_set(uint32 size);       // Sets ref_count and size
SEQ_OBJ*      new_seq(uint32 length);     // Sets ref_count, length, capacity, used_capacity and elems
BIN_REL_OBJ*  new_map(uint32 size);       // Sets ref_count and size, and clears rev_idxs
BIN_REL_OBJ*  new_bin_rel(uint32 size);   // Sets ref_count and size
TERN_REL_OBJ* new_tern_rel(uint32 size);  // Sets ref_count and size
TAG_OBJ*      new_tag_obj();              // Sets ref_count

SET_OBJ* shrink_set(SET_OBJ* set, uint32 new_size);

OBJ* new_obj_array(uint32 size);
void delete_obj_array(OBJ* buffer, uint32 size);
OBJ* resize_obj_array(OBJ* buffer, uint32 size, uint32 new_size);

uint32 *new_uint32_array(uint32 size);
void delete_uint32_array(uint32 *buffer, uint32 size);

int32* new_int32_array(uint32 size);
void delete_int32_array(int32* buffer, uint32 size);

char *new_byte_array(uint32 size);
void delete_byte_array(char* buffer, uint32 size);

void** new_ptr_array(uint32 size);
void delete_ptr_array(void** buffer, uint32 size);

void *new_void_array(uint32 size);
void delete_void_array(void* buffer, uint32 size);

//uint32 get_ref_count(OBJ);

//bool is_valid(OBJ);
//bool are_valid(OBJ* objs, uint32 count);

//////////////////////////////// mem-utils.cpp /////////////////////////////////

OBJ_TYPE get_logical_type(OBJ); //## SHOULD IT EXPOSE THE DIFFERENCE BETWEEN MAPS AND NON-MAP BINARY RELATIONS?

bool is_blank_obj(OBJ);
bool is_null_obj(OBJ);
bool is_symb(OBJ);
bool is_bool(OBJ);
bool is_int(OBJ);
bool is_float(OBJ);
bool is_seq(OBJ);
bool is_empty_seq(OBJ);
bool is_ne_seq(OBJ);
bool is_empty_rel(OBJ);
bool is_set(OBJ);
bool is_ne_set(OBJ);
bool is_bin_rel(OBJ);
bool is_ne_bin_rel(OBJ);
bool is_ne_map(OBJ);
bool is_tern_rel(OBJ);
bool is_ne_tern_rel(OBJ);
bool is_tag_obj(OBJ);

bool is_symb(OBJ, uint16);
bool is_int(OBJ, int64);

uint16 get_symb_idx(OBJ);
bool   get_bool(OBJ);
int64  get_int(OBJ);
double get_float(OBJ);
uint32 get_seq_length(OBJ);
uint16 get_tag_idx(OBJ);
OBJ    get_inner_obj(OBJ);

OBJ make_blank_obj();
OBJ make_null_obj();
OBJ make_empty_seq();
OBJ make_empty_rel();
OBJ make_symb(uint16 symb_idx);
OBJ make_bool(bool b);
OBJ make_int(uint64 value);
OBJ make_float(double value);
OBJ make_seq(SEQ_OBJ* ptr, uint32 length);
OBJ make_slice(SEQ_OBJ* ptr, MEM_LAYOUT mem_layout, uint32 offset, uint32 length);
OBJ make_set(SET_OBJ*);
OBJ make_bin_rel(BIN_REL_OBJ*);
OBJ make_tern_rel(TERN_REL_OBJ*);
OBJ make_log_map(BIN_REL_OBJ*);
OBJ make_map(BIN_REL_OBJ*);
OBJ make_tag_obj(uint16 tag_idx, OBJ obj);

// These functions exist in a limbo between the logical and physical world

uint32 get_seq_offset(OBJ);
OBJ* get_seq_buffer_ptr(OBJ);

// Purely physical representation functions

OBJ repoint_to_std_mem_copy(OBJ obj, void *new_ptr);

OBJ_TYPE get_physical_type(OBJ obj);

SEQ_OBJ*      get_seq_ptr(OBJ);
SET_OBJ*      get_set_ptr(OBJ);
BIN_REL_OBJ*  get_bin_rel_ptr(OBJ);
TERN_REL_OBJ* get_tern_rel_ptr(OBJ);
TAG_OBJ*      get_tag_obj_ptr(OBJ);

MEM_LAYOUT get_mem_layout(OBJ);

bool is_inline_obj(OBJ);
bool is_ref_obj(OBJ);
bool uses_try_mem(OBJ obj);
bool is_gc_obj(OBJ);

OBJ_TYPE get_ref_obj_type(OBJ);
REF_OBJ* get_ref_obj_ptr(OBJ);

bool are_shallow_eq(OBJ, OBJ);
int shallow_cmp(OBJ, OBJ);

//////////////////////////////// basic-ops.cpp /////////////////////////////////

bool inline_eq(OBJ obj1, OBJ obj2);
bool are_eq(OBJ obj1, OBJ obj2);
bool is_out_of_range(SET_ITER &it);
bool is_out_of_range(SEQ_ITER &it);
bool is_out_of_range(BIN_REL_ITER &it);
bool is_out_of_range(TERN_REL_ITER &it);
bool has_elem(OBJ set, OBJ elem);
bool has_key(OBJ rel, OBJ arg1);
bool has_field(OBJ rec, uint16 field_symb_idx);
bool has_pair(OBJ rel, OBJ arg1, OBJ arg2);
bool has_triple(OBJ rel, OBJ arg1, OBJ arg2, OBJ arg3);

int64 get_int_val(OBJ);
uint32 get_size(OBJ set);
int64 float_bits(OBJ);
int64 mantissa(OBJ);
int64 dec_exp(OBJ);
int64 rand_nat(int64 max);  // Non-deterministic
int64 unique_nat();         // Non-deterministic

OBJ obj_neg(OBJ);
OBJ at(OBJ seq, int64 idx);
OBJ get_tag(OBJ);
OBJ get_curr_obj(SET_ITER &it);
OBJ get_curr_obj(SEQ_ITER &it);
OBJ get_curr_left_arg(BIN_REL_ITER &it);
OBJ get_curr_right_arg(BIN_REL_ITER &it);
OBJ tern_rel_it_get_left_arg(TERN_REL_ITER &it);
OBJ tern_rel_it_get_mid_arg(TERN_REL_ITER &it);
OBJ tern_rel_it_get_right_arg(TERN_REL_ITER &it);
OBJ rand_set_elem(OBJ set);   // Non-deterministic

OBJ lookup(OBJ rel, OBJ key);
OBJ lookup_field(OBJ rec, uint16 field_symb_idx);

////////////////////////////////// instrs.cpp //////////////////////////////////

void init(STREAM &s);
void append(STREAM &s, OBJ obj);                // obj must be already reference-counted
OBJ build_seq(OBJ* elems, uint32 length);       // Objects in elems must be already reference-counted
OBJ build_seq(STREAM &s);
OBJ build_set(OBJ* elems, uint32 size);
OBJ build_set(STREAM &s);
OBJ build_tagged_obj(OBJ tag, OBJ obj);         // obj must be already reference-counted
// OBJ make_float(double val); // Already defined in mem_utils.cpp
OBJ neg_float(OBJ val);
OBJ add_floats(OBJ val1, OBJ val2);
OBJ sub_floats(OBJ val1, OBJ val2);
OBJ mult_floats(OBJ val1, OBJ val2);
OBJ div_floats(OBJ val1, OBJ val2);
OBJ exp_floats(OBJ val1, OBJ val2);
OBJ square_root(OBJ val);
OBJ floor(OBJ val);
OBJ ceiling(OBJ val);
OBJ int_to_float(OBJ val);
OBJ blank_array(int64 size);
OBJ get_seq_slice(OBJ seq, int64 idx_first, int64 len);
OBJ append_to_seq(OBJ seq, OBJ obj);            // Both seq and obj must already be reference counted
OBJ update_seq_at(OBJ seq, OBJ idx, OBJ value); // Value must be reference counted already
OBJ join_seqs(OBJ left, OBJ right);
OBJ rev_seq(OBJ seq);
void set_at(OBJ seq, uint32 idx, OBJ value);    // Value must be already reference counted
OBJ internal_sort(OBJ set);
OBJ parse_value(OBJ str);
OBJ print_value(OBJ);
void get_set_iter(SET_ITER &it, OBJ set);
void get_seq_iter(SEQ_ITER &it, OBJ seq);
void move_forward(SET_ITER &it);
void move_forward(SEQ_ITER &it);
void move_forward(BIN_REL_ITER &it);
void move_forward(TERN_REL_ITER &it);
void fail();
void runtime_check(OBJ cond);

OBJ build_const_uint8_seq(const uint8* buffer, uint32 len);
OBJ build_const_uint16_seq(const uint16* buffer, uint32 len);
OBJ build_const_uint32_seq(const uint32* buffer, uint32 len);
OBJ build_const_int8_seq(const int8* buffer, uint32 len);
OBJ build_const_int16_seq(const int16* buffer, uint32 len);
OBJ build_const_int32_seq(const int32* buffer, uint32 len);
OBJ build_const_int64_seq(const int64* buffer, uint32 len);

//////////////////////////////// bin-rel-obj.cpp ///////////////////////////////

OBJ build_bin_rel(OBJ *col1, OBJ *col2, uint32 size);
OBJ build_bin_rel(STREAM &strm1, STREAM &strm2);

OBJ build_map(OBJ* keys, OBJ* values, uint32 size);
OBJ build_map(STREAM &key_stream, STREAM &value_stream);

void get_bin_rel_iter(BIN_REL_ITER &it, OBJ rel);
void get_bin_rel_iter_0(BIN_REL_ITER &it, OBJ rel, OBJ arg1);
void get_bin_rel_iter_1(BIN_REL_ITER &it, OBJ rel, OBJ arg2);

/////////////////////////////// tern-rel-obj.cpp ///////////////////////////////

OBJ build_tern_rel(OBJ *col1, OBJ *col2, OBJ *col3, uint32 size);
OBJ build_tern_rel(STREAM &strm1, STREAM &strm2, STREAM &strm3);

void get_tern_rel_iter(TERN_REL_ITER &it, OBJ rel);
void get_tern_rel_iter_by(TERN_REL_ITER &it, OBJ rel, int col_idx, OBJ arg);
void get_tern_rel_iter_by(TERN_REL_ITER &it, OBJ rel, int major_col_idx, OBJ major_arg, OBJ minor_arg);

/////////////////////////////////// debug.cpp //////////////////////////////////

int get_call_stack_depth();

void push_call_info(const char* fn_name, uint32 arity, OBJ* params);
void pop_call_info();
void pop_try_mode_call_info(int depth);
void print_call_stack();
void dump_var(const char* name, OBJ value);
void print_assertion_failed_msg(const char* file, uint32 line, const char* text);

void soft_fail(const char *msg);
void impl_fail(const char *msg);
// void physical_fail();
void internal_fail();

////////////////////////////////// sorting.cpp /////////////////////////////////

void stable_index_sort(uint32 *index, OBJ *values, uint32 count);
void stable_index_sort(uint32 *index, OBJ *major_sort, OBJ *minor_sort, uint32 count);
void stable_index_sort(uint32 *index, OBJ *major_sort, OBJ *middle_sort, OBJ *minor_sort, uint32 count);

void index_sort(uint32 *index, OBJ *values, uint32 count);
void index_sort(uint32 *index, OBJ *major_sort, OBJ *minor_sort, uint32 count);
void index_sort(uint32 *index, OBJ *major_sort, OBJ *middle_sort, OBJ *minor_sort, uint32 count);

/////////////////////////////////// algs.cpp ///////////////////////////////////

uint32 sort_and_release_dups(OBJ* objs, uint32 size);
uint32 sort_and_check_no_dups(OBJ* keys, OBJ* values, uint32 size);
void sort_obj_array(OBJ* objs, uint32 len);

uint32 find_obj(OBJ* sorted_array, uint32 len, OBJ obj, bool &found); //## WHAT SHOULD THIS RETURN? ANY VALUE IN THE [0, 2^32-1] IS A VALID SEQUENCE INDEX, SO WHAT COULD BE USED TO REPRESENT "NOT FOUND"?
uint32 find_objs_range(OBJ *sorted_array, uint32 len, OBJ obj, uint32 &count);
uint32 find_idxs_range(uint32 *sorted_idx_array, OBJ *values, uint32 len, OBJ obj, uint32 &count);
uint32 find_objs_range(OBJ *major_col, OBJ *minor_col, uint32 len, OBJ major_arg, OBJ minor_arg, uint32 &count);
uint32 find_idxs_range(uint32 *index, OBJ *major_col, OBJ *minor_col, uint32 len, OBJ major_arg, OBJ minor_arg, uint32 &count);

int comp_objs(OBJ obj1, OBJ obj2);

/////////////////////////////// inter-utils.cpp ////////////////////////////////

void add_obj_to_cache(OBJ);
void release_all_cached_objs();

uint16 lookup_symb_idx(const char *, uint32);

const char *symb_to_raw_str(OBJ);

OBJ to_str(OBJ);
OBJ to_symb(OBJ);

OBJ extern_str_to_symb(const char *);

OBJ str_to_obj(const char* c_str);

char* obj_to_str(OBJ str_obj);
void obj_to_str(OBJ str_obj, char *buffer, uint32 size);

char* obj_to_byte_array(OBJ byte_seq_obj, uint32 &size);

uint64 char_buffer_size(OBJ str_obj);

//////////////////////////////// conversion.cpp ////////////////////////////////

OBJ convert_bool_seq(const bool *array, uint32 size);
OBJ convert_int32_seq(const int32 *array, uint32 size);
OBJ convert_int_seq(const int64 *array, uint32 size);
OBJ convert_float_seq(const double *array, uint32 size);
OBJ convert_text(const char *buffer);

// void export_as_c_string(OBJ obj, char *buffer, uint32 capacity);
uint32 export_as_bool_array(OBJ obj, bool *array, uint32 capacity);
uint32 export_as_long_long_array(OBJ obj, int64 *array, uint32 capacity);
uint32 export_as_float_array(OBJ obj, double *array, uint32 capacity);
void export_literal_as_c_string(OBJ obj, char *buffer, uint32 capacity);

///////////////////////////////// printing.cpp /////////////////////////////////

typedef enum {TEXT, SUB_START, SUB_END} EMIT_ACTION;

void print_obj(OBJ obj, void (*emit)(void *, const void *, EMIT_ACTION), void *data);

void print(OBJ);
void print_to_buffer_or_file(OBJ obj, char* buffer, uint32 max_size, const char* fname);
uint32 printed_obj(OBJ obj, char* buffer, uint32 max_size);
char *printed_obj(OBJ obj, char *alloc(void *, uint32), void *data);

////////////////////////////////// parsing.cpp /////////////////////////////////

bool parse(const char *text, uint32 size, OBJ *var, uint32 *error_offset);

///////////////////////////// os-interface-xxx.cpp /////////////////////////////

uint64 get_tick_count();   // Impure

////////////////////////////////// hashing.cpp /////////////////////////////////

uint32 compute_hash_code(OBJ obj);

//////////////////////////////// value-store.cpp ///////////////////////////////

void value_store_init(VALUE_STORE *store);
void value_store_cleanup(VALUE_STORE *store);

void value_store_updates_init(VALUE_STORE *store, VALUE_STORE_UPDATES *updates);
void value_store_updates_cleanup(VALUE_STORE_UPDATES *updates);

uint32 value_store_insert(VALUE_STORE *store, VALUE_STORE_UPDATES *updates, OBJ value);

void value_store_copy(VALUE_STORE *store, VALUE_STORE_UPDATES *updates);
void value_store_apply(VALUE_STORE *store, VALUE_STORE_UPDATES *updates);

void value_store_add_ref(VALUE_STORE *store, uint32 surr);
void value_store_release(VALUE_STORE *store, uint32 surr);

OBJ lookup_surrogate(VALUE_STORE *store, int64 surr);
int64 lookup_value(VALUE_STORE *store, OBJ value);

int64 lookup_value_ex(VALUE_STORE *store, VALUE_STORE_UPDATES *updates, OBJ value); //## FIND BETTER NAME...

OBJ *value_store_slot_array(VALUE_STORE *store);

//////////////////////////////// unary-table.cpp ///////////////////////////////

void unary_table_init(UNARY_TABLE *table);
void unary_table_cleanup(UNARY_TABLE *table);

void unary_table_updates_init(UNARY_TABLE_UPDATES *table);
void unary_table_updates_cleanup(UNARY_TABLE_UPDATES *table);

bool unary_table_contains(UNARY_TABLE *table, uint32 value);

void unary_table_insert(UNARY_TABLE_UPDATES *updates, uint32 value);

void unary_table_delete(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates, uint32 value);
void unary_table_clear(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates);

bool unary_table_updates_check(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates);
void unary_table_updates_apply(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates, VALUE_STORE *vs);
void unary_table_updates_finish(UNARY_TABLE_UPDATES *updates, VALUE_STORE *vs);

void unary_table_get_iter(UNARY_TABLE *table, UNARY_TABLE_ITER *iter);
void unary_table_iter_next(UNARY_TABLE_ITER *iter);

bool unary_table_iter_is_out_of_range(UNARY_TABLE_ITER *iter);

uint32 unary_table_iter_get_field(UNARY_TABLE_ITER *iter);

OBJ copy_unary_table(UNARY_TABLE *table, VALUE_STORE *vs);

void set_unary_table(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates, VALUE_STORE *vs, VALUE_STORE_UPDATES *vsu, OBJ set);

/////////////////////////////// binary-table.cpp ///////////////////////////////

void binary_table_init(BINARY_TABLE *table);
void binary_table_cleanup(BINARY_TABLE *table);

void binary_table_updates_init(BINARY_TABLE_UPDATES *updates);
void binary_table_updates_cleanup(BINARY_TABLE_UPDATES *updates);

bool binary_table_contains(BINARY_TABLE *table, uint32 left_val, uint32 right_val);

void binary_table_delete(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, uint32 left_val, uint32 right_val);
void binary_table_delete_by_col_0(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, uint32 value);
void binary_table_delete_by_col_1(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, uint32 value);
void binary_table_clear(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates);

void binary_table_insert(BINARY_TABLE_UPDATES *updates, uint32 left_val, uint32 right_val);

bool binary_table_updates_check_0(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates);
bool binary_table_updates_check_1(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates);
bool binary_table_updates_check_0_1(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates);

void binary_table_updates_apply(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, VALUE_STORE *vs0, VALUE_STORE *vs1);
void binary_table_updates_finish(BINARY_TABLE_UPDATES *updates, VALUE_STORE *vs0, VALUE_STORE *vs1);

void binary_table_get_iter_by_col_0(BINARY_TABLE *table, BINARY_TABLE_ITER *iter, uint32 value);
void binary_table_get_iter_by_col_1(BINARY_TABLE *table, BINARY_TABLE_ITER *iter, uint32 value);
void binary_table_get_iter(BINARY_TABLE *table, BINARY_TABLE_ITER *iter);

bool binary_table_iter_is_out_of_range(BINARY_TABLE_ITER *iter);

uint32 binary_table_iter_get_left_field(BINARY_TABLE_ITER *iter);
uint32 binary_table_iter_get_right_field(BINARY_TABLE_ITER *iter);

void binary_table_iter_next(BINARY_TABLE_ITER *iter);

OBJ copy_binary_table(BINARY_TABLE *table, VALUE_STORE *vs1, VALUE_STORE *vs2, bool flip_cols);

void set_binary_table(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, VALUE_STORE *vs1, VALUE_STORE *vs2,
  VALUE_STORE_UPDATES *vsu1, VALUE_STORE_UPDATES *vsu2, OBJ rel, bool flip_cols);

/////////////////////////////// ternary-table.cpp ///////////////////////////////

void ternary_table_init(TERNARY_TABLE *table);
void ternary_table_cleanup(TERNARY_TABLE *table);

void ternary_table_updates_init(TERNARY_TABLE_UPDATES *updates);
void ternary_table_updates_cleanup(TERNARY_TABLE_UPDATES *updates);

bool ternary_table_contains(TERNARY_TABLE *table, uint32 left_val, uint32 middle_val, uint32 right_val);

void ternary_table_delete(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates, uint32 left_val, uint32 middle_val, uint32 right_val);
void ternary_table_delete_by_cols_01(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates, uint32 value0, uint32 value1);
void ternary_table_delete_by_cols_02(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates, uint32 value0, uint32 value2);
void ternary_table_delete_by_cols_12(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates, uint32 value1, uint32 value2);
void ternary_table_delete_by_col_0(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates, uint32 value);
void ternary_table_delete_by_col_1(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates, uint32 value);
void ternary_table_delete_by_col_2(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates, uint32 value);
void ternary_table_clear(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates);

void ternary_table_insert(TERNARY_TABLE_UPDATES *updates, uint32 left_val, uint32 middle_val, uint32 right_val);

bool ternary_table_updates_check_01(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates);
bool ternary_table_updates_check_01_2(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates);
bool ternary_table_updates_check_01_12(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates);
bool ternary_table_updates_check_01_12_20(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates);

void ternary_table_updates_apply(TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates, VALUE_STORE *vs0, VALUE_STORE *vs1, VALUE_STORE *vs2);
void ternary_table_updates_finish(TERNARY_TABLE_UPDATES *updates, VALUE_STORE *vs0, VALUE_STORE *vs1, VALUE_STORE *vs2);

void ternary_table_get_iter_by_cols_01(TERNARY_TABLE *table, TERNARY_TABLE_ITER *iter, uint32 value0, uint32 value1);
void ternary_table_get_iter_by_cols_02(TERNARY_TABLE *table, TERNARY_TABLE_ITER *iter, uint32 value0, uint32 value2);
void ternary_table_get_iter_by_cols_12(TERNARY_TABLE *table, TERNARY_TABLE_ITER *iter, uint32 value1, uint32 value2);
void ternary_table_get_iter_by_col_0(TERNARY_TABLE *table, TERNARY_TABLE_ITER *iter, uint32 value);
void ternary_table_get_iter_by_col_1(TERNARY_TABLE *table, TERNARY_TABLE_ITER *iter, uint32 value);
void ternary_table_get_iter_by_col_2(TERNARY_TABLE *table, TERNARY_TABLE_ITER *iter, uint32 value);
void ternary_table_get_iter(TERNARY_TABLE *table, TERNARY_TABLE_ITER *iter);

bool ternary_table_iter_is_out_of_range(TERNARY_TABLE_ITER *iter);

uint32 ternary_table_iter_get_left_field(TERNARY_TABLE_ITER *iter);
uint32 ternary_table_iter_get_middle_field(TERNARY_TABLE_ITER *iter);
uint32 ternary_table_iter_get_right_field(TERNARY_TABLE_ITER *iter);

void ternary_table_iter_next(TERNARY_TABLE_ITER *iter);

OBJ copy_ternary_table(TERNARY_TABLE *table, VALUE_STORE *vs1, VALUE_STORE *vs2, VALUE_STORE *vs3, int idx1, int idx2, int idx3);

void set_ternary_table(
  TERNARY_TABLE *table, TERNARY_TABLE_UPDATES *updates,
  VALUE_STORE *vs1, VALUE_STORE *vs2, VALUE_STORE *vs3,
  VALUE_STORE_UPDATES *vsu1, VALUE_STORE_UPDATES *vsu2, VALUE_STORE_UPDATES *vsu3,
  OBJ rel, int idx1, int idx2, int idx3
);
