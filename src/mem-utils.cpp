#include "lib.h"


const uint32 SEQ_BUFFER_FIELD_OFFSET = offsetof(SEQ_OBJ, buffer);

////////////////////////////////////////////////////////////////////////////////

#define MASK(S, W)      (((1ULL << (W)) - 1) << (S))
#define MAKE(V, S)      (((uint64) (V)) << (S))
#define GET(V, S, W)    (((V) & MASK(S, W)) >> (S))
#define CLEAR(V, M)     ((V) & ~(M))

////////////////////////////////////////////////////////////////////////////////

const int TYPE_SHIFT          = 56;
const int MEM_LAYOUT_SHIFT    = 60;
const int TAGS_COUNT_SHIFT    = 62;

const int TYPE_WIDTH          = 4;
const int MEM_LAYOUT_WIDTH    = 2;
const int TAGS_COUNT_WIDTH    = 2;

const int SYMB_IDX_SHIFT      = 0;
const int LENGTH_SHIFT        = 0;
const int INNER_TAG_SHIFT     = 16;
const int TAG_SHIFT           = 32;
const int OFFSET_SHIFT        = 28;

const int SYMB_IDX_WIDTH      = 16;
const int LENGTH_WIDTH        = 28;
const int TAG_WIDTH           = 16;
const int OFFSET_WIDTH        = 28;

const uint64 TYPE_MASK        = MASK(TYPE_SHIFT, TYPE_WIDTH);
const uint64 MEM_LAYOUT_MASK  = MASK(MEM_LAYOUT_SHIFT, MEM_LAYOUT_WIDTH);
const uint64 TAGS_COUNT_MASK  = MASK(TAGS_COUNT_SHIFT, TAGS_COUNT_WIDTH);

const uint64 SYMB_IDX_MASK    = MASK(SYMB_IDX_SHIFT, SYMB_IDX_WIDTH);
const uint64 LENGTH_MASK      = MASK(LENGTH_SHIFT, LENGTH_WIDTH);
const uint64 INNER_TAG_MASK   = MASK(INNER_TAG_SHIFT, TAG_WIDTH);
const uint64 TAG_MASK         = MASK(TAG_SHIFT, TAG_WIDTH);
const uint64 OFFSET_MASK      = MASK(OFFSET_SHIFT, OFFSET_WIDTH);

////////////////////////////////////////////////////////////////////////////////

#define MAKE_TYPE(T)        MAKE(T, TYPE_SHIFT)
#define MAKE_MEM_LAYOUT(L)  MAKE(L, MEM_LAYOUT_SHIFT)
#define MAKE_OFFSET(O)      MAKE(O, OFFSET_SHIFT)
#define MAKE_LENGTH(L)      MAKE(L, LENGTH_SHIFT)
#define MAKE_TAGS_COUNT(C)  MAKE(C, TAGS_COUNT_SHIFT)
#define MAKE_INNER_TAG(T)   MAKE(T, INNER_TAG_SHIFT)
#define MAKE_SYMB_IDX(I)    MAKE(I, SYMB_IDX_SHIFT)
#define MAKE_TAG(T)         MAKE(T, TAG_SHIFT)

#define STD_MEM_LAYOUT      MAKE_MEM_LAYOUT(STD_MEM)
#define TRY_MEM_LAYOUT      MAKE_MEM_LAYOUT(TRY_MEM)

////////////////////////////////////////////////////////////////////////////////

const uint64 BLANK_OBJ_MASK       = MAKE_TYPE(TYPE_BLANK_OBJ);
const uint64 NULL_OBJ_MASK        = MAKE_TYPE(TYPE_NULL_OBJ);

const uint64 SYMBOL_BASE_MASK     = MAKE_TYPE(TYPE_SYMBOL);
const uint64 INTEGER_MASK         = MAKE_TYPE(TYPE_INTEGER);
const uint64 FLOAT_MASK           = MAKE_TYPE(TYPE_FLOAT);
const uint64 EMPTY_SEQ_MASK       = MAKE_TYPE(TYPE_SEQUENCE);
const uint64 NE_SEQ_BASE_MASK     = MAKE_TYPE(TYPE_SEQUENCE) | STD_MEM_LAYOUT;
const uint64 EMPTY_REL_MASK       = MAKE_TYPE(TYPE_SET);
const uint64 NE_SET_MASK          = MAKE_TYPE(TYPE_SET)      | STD_MEM_LAYOUT;
const uint64 NE_BIN_REL_MASK      = MAKE_TYPE(TYPE_BIN_REL)  | STD_MEM_LAYOUT;
const uint64 NE_MAP_MASK          = MAKE_TYPE(TYPE_MAP)      | STD_MEM_LAYOUT;
const uint64 NE_LOG_MAP_MASK      = MAKE_TYPE(TYPE_LOG_MAP)  | STD_MEM_LAYOUT;
const uint64 NE_TERN_REL_MASK     = MAKE_TYPE(TYPE_TERN_REL) | STD_MEM_LAYOUT;
const uint64 TAG_OBJ_MASK         = MAKE_TYPE(TYPE_TAG_OBJ)  | STD_MEM_LAYOUT;

const uint64 TRY_STATE_NE_SEQ_BASE_MASK   = MAKE_TYPE(TYPE_SEQUENCE) | TRY_MEM_LAYOUT;
const uint64 TRY_STATE_NE_SET_MASK        = MAKE_TYPE(TYPE_SET)      | TRY_MEM_LAYOUT;
const uint64 TRY_STATE_NE_BIN_REL_MASK    = MAKE_TYPE(TYPE_BIN_REL)  | TRY_MEM_LAYOUT;
const uint64 TRY_STATE_NE_MAP_MASK        = MAKE_TYPE(TYPE_MAP)      | TRY_MEM_LAYOUT;
const uint64 TRY_STATE_NE_LOG_MAP_MASK    = MAKE_TYPE(TYPE_LOG_MAP)  | TRY_MEM_LAYOUT;
const uint64 TRY_STATE_NE_TERN_REL_MASK   = MAKE_TYPE(TYPE_TERN_REL) | TRY_MEM_LAYOUT;
const uint64 TRY_STATE_TAG_OBJ_MASK       = MAKE_TYPE(TYPE_TAG_OBJ)  | TRY_MEM_LAYOUT;

const uint64 SET_LOG_MASK       = MAKE_TYPE(TYPE_SET);
const uint64 BIN_REL_LOG_MASK   = MAKE_TYPE(TYPE_BIN_REL);
const uint64 MAP_LOG_MASK       = MAKE_TYPE(TYPE_MAP);
const uint64 LOG_MAP_LOG_MASK   = MAKE_TYPE(TYPE_LOG_MAP);
const uint64 TERN_REL_LOG_MASK  = MAKE_TYPE(TYPE_TERN_REL);

////////////////////////////////////////////////////////////////////////////////

void append_bits(uint64 word, int leftmost, int count, char *str) {
  assert(count > 0);
  assert(leftmost >= 0 & leftmost < 64);
  assert(leftmost - count + 1 >= 0);

  char *buff = str + strlen(str);

  for (int i=0 ; i < count ; i++)
    buff[i] = '0' + ((word >> (leftmost - i)) & 1);

  buff[count] = '\0';
}

void print_ref(OBJ obj) {
  static int frags[] = {2, 2, 4, 8, 16, 16, 16};
  static const char *type_names[] = {
    "TYPE_BLANK_OBJ",
    "TYPE_NULL_OBJ",
    "TYPE_SYMBOL",
    "TYPE_INTEGER",
    "TYPE_FLOAT",
    "TYPE_SEQUENCE",
    "TYPE_SET",
    "TYPE_BIN_REL",
    "TYPE_TERN_REL",
    "TYPE_TAG_OBJ",
    "TYPE_SLICE",
    "TYPE_MAP",
    "TYPE_LOG_MAP"
  };

  char buffer[256];
  buffer[0] = '\0';

  int bit_idx = 63;
  for (int i=0 ; i < sizeof(frags)/sizeof(int) ; i++) {
    if (i > 0)
      strcat(buffer, " ");
    append_bits(obj.extra_data, bit_idx, frags[i], buffer);
    bit_idx -= frags[i];
  }

  assert(bit_idx == -1);

  OBJ_TYPE phys_type = get_physical_type(obj);
  const char *type_name = phys_type < sizeof(type_names)/sizeof(char*) ? type_names[phys_type] : "INVALID_TYPE";

  printf("%s - (%s)\n", buffer, type_name);
  fflush(stdout);
}

////////////////////////////////////////////////////////////////////////////////

OBJ_TYPE get_physical_type(OBJ obj) {
  uint64 field = GET(obj.extra_data, TYPE_SHIFT, TYPE_WIDTH);
  return (OBJ_TYPE) field;
  // return (OBJ_TYPE) GET(obj.extra_data, TYPE_SHIFT, TYPE_WIDTH);
}

MEM_LAYOUT get_mem_layout(OBJ obj) {
  return (MEM_LAYOUT) GET(obj.extra_data, MEM_LAYOUT_SHIFT, MEM_LAYOUT_WIDTH);
}

uint32 get_tags_count(OBJ obj) {
  return GET(obj.extra_data, TAGS_COUNT_SHIFT, TAGS_COUNT_WIDTH); // Masking is actually unnecessary here
}

////////////////////////////////////////////////////////////////////////////////

OBJ_TYPE get_logical_type(OBJ obj) {
  OBJ_TYPE type = get_physical_type(obj);

  if (type == TYPE_SLICE)
    return TYPE_SEQUENCE;

  if (get_tags_count(obj) > 0)
    return TYPE_TAG_OBJ;

  if (type == TYPE_MAP | type == TYPE_LOG_MAP)
    return TYPE_BIN_REL;

  return type;
}

////////////////////////////////////////////////////////////////////////////////

uint64 get_log_mask(OBJ obj) {
  return CLEAR(obj.extra_data, MEM_LAYOUT_MASK);
}

////////////////////////////////////////////////////////////////////////////////

OBJ make_blank_obj() {
  OBJ obj;
  obj.core_data.int_ = 0;
  obj.extra_data = BLANK_OBJ_MASK;
  return obj;
}

OBJ make_null_obj() {
  OBJ obj;
  obj.core_data.int_ = 0;
  obj.extra_data = NULL_OBJ_MASK;
  return obj;
}

OBJ make_symb(uint16 symb_idx) {
  OBJ obj;
  obj.core_data.int_ = 0;
  obj.extra_data = symb_idx | SYMBOL_BASE_MASK;

  // assert(obj.extra_data.std.symb_idx    == symb_idx);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_SYMBOL);
  // assert(obj.extra_data.std.mem_layout  == 0);
  // assert(obj.extra_data.std.num_tags    == 0);
  // assert(is_symb(obj, symb_idx));

  return obj;
}

OBJ make_bool(bool b) {
  OBJ obj;
  obj.core_data.int_ = 0;
  obj.extra_data = (b ? symb_idx_true : symb_idx_false) | SYMBOL_BASE_MASK;

  // assert(obj.extra_data.std.symb_idx    == (b ? symb_idx_true : symb_idx_false));
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_SYMBOL);
  // assert(obj.extra_data.std.mem_layout  == 0);
  // assert(obj.extra_data.std.num_tags    == 0);
  // assert(is_symb(obj, (b ? symb_idx_true : symb_idx_false)));

  return obj;
}

OBJ make_int(uint64 value) {
  OBJ obj;
  obj.core_data.int_ = value;
  obj.extra_data = INTEGER_MASK;

  // assert(obj.extra_data.std.symb_idx    == 0);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_INTEGER);
  // assert(obj.extra_data.std.mem_layout  == 0);
  // assert(obj.extra_data.std.num_tags    == 0);
  // assert(is_int(obj, value));

  return obj;
}

OBJ make_float(double value) {
  OBJ obj;
  obj.core_data.float_ = value;
  obj.extra_data = FLOAT_MASK;

  // assert(obj.extra_data.std.symb_idx    == 0);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_FLOAT);
  // assert(obj.extra_data.std.mem_layout  == 0);
  // assert(obj.extra_data.std.num_tags    == 0);

  return obj;
}

OBJ make_seq(SEQ_OBJ *ptr, uint32 length) {
  assert(length == 0 || (ptr != NULL & length <= ptr->size));

  OBJ obj;
  if (length == 0) {
    obj.core_data.ptr = NULL;
    obj.extra_data = EMPTY_SEQ_MASK;
  }
  else {
    obj.core_data.ptr = ptr->buffer;
    obj.extra_data = length | (is_in_try_state() ? TRY_STATE_NE_SEQ_BASE_MASK : NE_SEQ_BASE_MASK);
  }

  // assert(obj.extra_data.seq.length      == length);
  // assert(obj.extra_data.seq.tag         == 0);
  // assert(obj.extra_data.seq.unused_byte == 0);
  // assert(obj.extra_data.seq.type        == TYPE_SEQUENCE);
  // assert(obj.extra_data.seq.mem_layout  == (length > 0 ? (is_in_try_state() ? 2 : 1) : 0));
  // assert(obj.extra_data.seq.num_tags    == 0);

  assert(length == 0 || get_seq_ptr(obj) == ptr);

  return obj;
}

OBJ make_slice(SEQ_OBJ *ptr, MEM_LAYOUT mem_layout, uint32 offset, uint32 length) {
  assert(ptr != NULL & ((uint64) offset) + ((uint64) length) <= ptr->size);
  assert(offset <= 0xFFFFFFF);

  // if (offset > 0xFFFFFFF)
  //   impl_fail("Currently subsequences cannot start at offsets greater that 2^28-1");

  // If the offset is 0, then we can just create a normal sequence
  // If the length is 0, we must create an empty sequence, which is again a normal sequence
  if (offset == 0 | length == 0) {
    OBJ obj = make_seq(ptr, length);
    obj.extra_data = get_log_mask(obj) | MAKE_MEM_LAYOUT(mem_layout);
    // assert(obj.extra_data.slice.mem_layout == mem_layout);
    return obj;
  }

  OBJ obj;
  obj.core_data.ptr = ptr->buffer + offset;
  obj.extra_data = MAKE_LENGTH(length) | MAKE_OFFSET(offset) | MAKE_TYPE(TYPE_SLICE) | MAKE_MEM_LAYOUT(mem_layout);

  // assert(obj.extra_data.slice.length == length);
  // assert(obj.extra_data.slice.offset == offset);
  // assert(obj.extra_data.slice.type == TYPE_SLICE);
  // assert(obj.extra_data.slice.mem_layout == mem_layout);
  // assert(obj.extra_data.slice.num_tags == 0);

  assert(length == 0 || get_seq_ptr(obj) == ptr);

  return obj;
}

OBJ make_empty_seq() {
  return make_seq(NULL, 0);
}

OBJ make_set(SET_OBJ *ptr) {
  OBJ obj;
  obj.core_data.ptr = ptr;
  obj.extra_data = ptr == NULL ? EMPTY_REL_MASK : (is_in_try_state() ? TRY_STATE_NE_SET_MASK : NE_SET_MASK);

  // assert(obj.extra_data.std.symb_idx    == 0);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_SET);
  // assert(obj.extra_data.std.mem_layout  == (ptr != NULL ? (is_in_try_state() ? 2 : 1) : 0));
  // assert(obj.extra_data.std.num_tags    == 0);

  return obj;
}

OBJ make_empty_rel() {
  return make_set(NULL);
}

OBJ make_bin_rel(BIN_REL_OBJ *ptr) {
  assert(ptr != NULL);

  OBJ obj;
  obj.core_data.ptr = ptr;
  obj.extra_data = is_in_try_state() ? TRY_STATE_NE_BIN_REL_MASK : NE_BIN_REL_MASK;

  // assert(obj.extra_data.std.symb_idx    == 0);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_BIN_REL);
  // assert(obj.extra_data.std.mem_layout  == (is_in_try_state() ? 2 : 1));
  // assert(obj.extra_data.std.num_tags    == 0);

  return obj;
}

OBJ make_log_map(BIN_REL_OBJ *ptr) {
  assert(ptr != NULL);

  OBJ obj;
  obj.core_data.ptr = ptr;
  obj.extra_data = is_in_try_state() ? TRY_STATE_NE_LOG_MAP_MASK : NE_LOG_MAP_MASK;

  // assert(obj.extra_data.std.symb_idx    == 0);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_LOG_MAP);
  // assert(obj.extra_data.std.mem_layout  == (is_in_try_state() ? 2 : 1));
  // assert(obj.extra_data.std.num_tags    == 0);

  return obj;
}

OBJ make_map(BIN_REL_OBJ *ptr) {
  assert(ptr != NULL);

  OBJ obj;
  obj.core_data.ptr = ptr;
  obj.extra_data = is_in_try_state() ? TRY_STATE_NE_MAP_MASK : NE_MAP_MASK;

  // assert(obj.extra_data.std.symb_idx    == 0);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_MAP);
  // assert(obj.extra_data.std.mem_layout  == (is_in_try_state() ? 2 : 1));
  // assert(obj.extra_data.std.num_tags    == 0);

  return obj;
}

OBJ make_tern_rel(TERN_REL_OBJ *ptr) {
  assert(ptr != NULL);

  OBJ obj;
  obj.core_data.ptr = ptr;
  obj.extra_data = is_in_try_state() ? TRY_STATE_NE_TERN_REL_MASK : NE_TERN_REL_MASK;

  // assert(obj.extra_data.std.symb_idx    == 0);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_TERN_REL);
  // assert(obj.extra_data.std.mem_layout  == (is_in_try_state() ? 2 : 1));
  // assert(obj.extra_data.std.num_tags    == 0);

  return obj;
}

OBJ make_tag_obj(TAG_OBJ *ptr) {
  OBJ obj;
  obj.core_data.ptr = ptr;
  obj.extra_data = (is_in_try_state() ? TRY_STATE_TAG_OBJ_MASK : TAG_OBJ_MASK);

  // assert(obj.extra_data.std.symb_idx    == 0);
  // assert(obj.extra_data.std.inner_tag   == 0);
  // assert(obj.extra_data.std.tag         == 0);
  // assert(obj.extra_data.std.unused_byte == 0);
  // assert(obj.extra_data.std.type        == TYPE_TAG_OBJ);
  // assert(obj.extra_data.std.mem_layout  == (is_in_try_state() ? 2 : 1));
  // assert(obj.extra_data.std.num_tags    == 0);

  return obj;
}

OBJ make_ref_tag_obj(uint16 tag_idx, OBJ obj) {
  TAG_OBJ *tag_obj = new_tag_obj();
  tag_obj->tag_idx = tag_idx;
  tag_obj->obj = obj;
  return make_tag_obj(tag_obj);
}

OBJ make_tag_obj(uint16 tag_idx, OBJ obj) {
  OBJ_TYPE type = get_physical_type(obj);

  if (type == TYPE_SEQUENCE) {
    if (get_tags_count(obj) == 0) {
      // No need to clear anything, both fields are already blank
      obj.extra_data |= MAKE_TAG(tag_idx) | MAKE_TAGS_COUNT(1);
      // obj.extra_data.seq.tag = tag_idx;
      // obj.extra_data.seq.num_tags = 1;
      return obj;
    }
  }
  else if (type != TYPE_SLICE) {
    uint8 tags_count = get_tags_count(obj);
    if (tags_count < 2) {
      uint16 curr_tag_idx = GET(obj.extra_data, TAG_SHIFT, TAG_WIDTH);
      uint64 mask_diff = MAKE_INNER_TAG(curr_tag_idx) | MAKE_TAG(tag_idx) | MAKE_TAGS_COUNT(tags_count + 1);
      // No need to clear the inner tag here, it's already 0
      obj.extra_data = CLEAR(obj.extra_data, TAG_MASK | TAGS_COUNT_MASK) | mask_diff;
      // obj.extra_data.std.inner_tag = obj.extra_data.std.tag;
      // obj.extra_data.std.tag = tag_idx;
      // obj.extra_data.std.num_tags = tags_count + 1;
      return obj;
    }
  }

  return make_ref_tag_obj(tag_idx, obj);
}

////////////////////////////////////////////////////////////////////////////////

//## IF WE DECIDE TO OPTIMIZE THE COPYING OF SEQUENCES AND/OR SLICES, THIS WILL STOP WORKING
OBJ repoint_to_std_mem_copy(OBJ obj, void *new_ptr) {
  obj.core_data.ptr = new_ptr;
  obj.extra_data = CLEAR(obj.extra_data, MEM_LAYOUT_MASK) | MAKE_MEM_LAYOUT(new_ptr != NULL ? 1 : 0);
  assert(obj.core_data.ptr != NULL || get_mem_layout(obj) == 0);
  return obj;
}

////////////////////////////////////////////////////////////////////////////////

uint16 get_symb_idx(OBJ obj) {
  assert(is_symb(obj));
  return GET(obj.extra_data, SYMB_IDX_SHIFT, SYMB_IDX_WIDTH);
}

bool get_bool(OBJ obj) {
  assert(is_bool(obj));
  return get_symb_idx(obj) == symb_idx_true;
}

int64 get_int(OBJ obj) {
  assert(is_int(obj));
  return obj.core_data.int_;
}

double get_float(OBJ obj) {
  assert(is_float(obj));
  return obj.core_data.float_;
}

uint32 get_seq_length(OBJ seq) {
  assert(is_seq(seq));
  // // The length field should overlap for sequences and slices,
  // // there should be no need to check the specific type of sequence
  // assert(seq.extra_data.seq.length == seq.extra_data.slice.length);
  return GET(seq.extra_data, LENGTH_SHIFT, LENGTH_WIDTH);
}

uint32 get_seq_offset(OBJ seq) {
  assert(is_seq(seq));
  return get_physical_type(seq) == TYPE_SLICE ? GET(seq.extra_data, OFFSET_SHIFT, OFFSET_WIDTH) : 0;
}

uint16 get_tag_idx(OBJ obj) {
  assert(is_tag_obj(obj));
  assert(get_physical_type(obj) != TYPE_SLICE);

  if (get_tags_count(obj) != 0)
    return GET(obj.extra_data, TAG_SHIFT, TAG_WIDTH);
  else
    return ((TAG_OBJ *) obj.core_data.ptr)->tag_idx;
}

OBJ get_inner_obj(OBJ obj) {
  assert(is_tag_obj(obj));

  OBJ_TYPE type = get_physical_type(obj);
  assert(type != TYPE_SLICE);

  if (type == TYPE_SEQUENCE) {
    assert(get_tags_count(obj) == 1);
    obj.extra_data = CLEAR(obj.extra_data, TAG_MASK | TAGS_COUNT_MASK);
    // obj.extra_data.seq.tag = 0;
    // obj.extra_data.seq.num_tags = 0;
    return obj;
  }

  uint8 tags_count = get_tags_count(obj);
  if (tags_count > 0) {
    uint16 inner_tag = GET(obj.extra_data, INNER_TAG_SHIFT, TAG_WIDTH);
    uint64 cleared_extra_data = CLEAR(obj.extra_data, INNER_TAG_MASK | TAG_MASK | TAGS_COUNT_MASK);
    obj.extra_data = cleared_extra_data | MAKE_TAG(inner_tag) | MAKE_TAGS_COUNT(tags_count-1);
    // obj.extra_data.std.tag = obj.extra_data.std.inner_tag;
    // obj.extra_data.std.inner_tag = 0;
    // obj.extra_data.std.num_tags = tags_count - 1;
    return obj;
  }

  return ((TAG_OBJ *) obj.core_data.ptr)->obj;
}

////////////////////////////////////////////////////////////////////////////////

OBJ *get_seq_buffer_ptr(OBJ obj) {
  assert(is_ne_seq(obj));
  return (OBJ *) obj.core_data.ptr;
}

////////////////////////////////////////////////////////////////////////////////

SEQ_OBJ* get_seq_ptr(OBJ obj) {
  assert(get_physical_type(obj) == TYPE_SEQUENCE || get_physical_type(obj) == TYPE_SLICE);
  assert(obj.core_data.ptr != NULL);

  char *buffer_ptr = (char *) obj.core_data.ptr;
  // Here I cannot simply use get_seq_offset(obj) because that function asserts the object has to be an untagged sequence
  uint32 slice_offset = get_physical_type(obj) == TYPE_SLICE ? GET(obj.extra_data, OFFSET_SHIFT, OFFSET_WIDTH) : 0;

  return (SEQ_OBJ *) (buffer_ptr - (SEQ_BUFFER_FIELD_OFFSET + slice_offset * sizeof(OBJ)));
}

SET_OBJ* get_set_ptr(OBJ obj) {
  assert(get_physical_type(obj) == TYPE_SET & obj.core_data.ptr != NULL);
  return (SET_OBJ *) obj.core_data.ptr;
}

BIN_REL_OBJ *get_bin_rel_ptr(OBJ obj) {
  assert(get_physical_type(obj) == TYPE_BIN_REL | get_physical_type(obj) == TYPE_LOG_MAP | get_physical_type(obj) == TYPE_MAP);
  assert(obj.core_data.ptr != NULL);
  return (BIN_REL_OBJ *) obj.core_data.ptr;
}

TERN_REL_OBJ *get_tern_rel_ptr(OBJ obj) {
  assert(get_physical_type(obj) == TYPE_TERN_REL & obj.core_data.ptr != NULL);
  return (TERN_REL_OBJ *) obj.core_data.ptr;
}

TAG_OBJ *get_tag_obj_ptr(OBJ obj) {
  assert(get_physical_type(obj) == TYPE_TAG_OBJ & obj.core_data.ptr != NULL);
  return (TAG_OBJ *) obj.core_data.ptr;
}

////////////////////////////////////////////////////////////////////////////////

bool is_blank_obj(OBJ obj) {
  return get_physical_type(obj) == TYPE_BLANK_OBJ;
}

bool is_null_obj(OBJ obj) {
  return get_physical_type(obj) == TYPE_NULL_OBJ;
}

bool is_symb(OBJ obj) {
  return (obj.extra_data >> 16) == (SYMBOL_BASE_MASK >> 16);
}

bool is_bool(OBJ obj) {
  assert(symb_idx_false == 0 & symb_idx_true == 1); // The correctness of the body depends on this assumption
  return (obj.extra_data >> 1) == (SYMBOL_BASE_MASK >> 1);
}

bool is_int(OBJ obj) {
  return obj.extra_data == INTEGER_MASK;
}

bool is_float(OBJ obj) {
  return obj.extra_data == FLOAT_MASK;
}

bool is_seq(OBJ obj) {
  OBJ_TYPE physical_type = get_physical_type(obj);
  return (physical_type == TYPE_SEQUENCE & get_tags_count(obj) == 0) | physical_type == TYPE_SLICE;
}

bool is_empty_seq(OBJ obj) {
  return obj.extra_data == EMPTY_SEQ_MASK;
}

bool is_ne_seq(OBJ obj) {
  // return ((obj.extra_data.word >> 32) == (NE_SEQ_BASE_MASK >> 32)) | (get_physical_type(obj) == TYPE_SLICE);
  return is_seq(obj) & !is_empty_seq(obj);
}

bool is_empty_rel(OBJ obj) {
  return obj.extra_data == EMPTY_REL_MASK;
}

bool is_ne_set(OBJ obj) {
  return obj.extra_data == NE_SET_MASK | obj.extra_data == TRY_STATE_NE_SET_MASK;
}

bool is_set(OBJ obj) {
  return get_log_mask(obj) == SET_LOG_MASK;
}

bool is_ne_bin_rel(OBJ obj) {
  uint64 log_mask = get_log_mask(obj);
  return log_mask == BIN_REL_LOG_MASK | log_mask == LOG_MAP_LOG_MASK | log_mask == MAP_LOG_MASK;
}

bool is_ne_map(OBJ obj) {
  uint64 log_mask = get_log_mask(obj);
  return log_mask == MAP_LOG_MASK | log_mask == LOG_MAP_LOG_MASK;
}

bool is_bin_rel(OBJ obj) {
  return is_empty_rel(obj) | is_ne_bin_rel(obj);
}

bool is_ne_tern_rel(OBJ obj) {
  uint64 extra_data = obj.extra_data;
  return extra_data == NE_TERN_REL_MASK | extra_data == TRY_STATE_NE_TERN_REL_MASK;
}

bool is_tern_rel(OBJ obj) {
  return is_empty_rel(obj) | is_ne_tern_rel(obj);
}

bool is_tag_obj(OBJ obj) {
  return get_tags_count(obj) != 0 | get_physical_type(obj) == TYPE_TAG_OBJ;
}

////////////////////////////////////////////////////////////////////////////////

bool is_symb(OBJ obj, uint16 symb_idx) {
  return obj.extra_data == (symb_idx | SYMBOL_BASE_MASK);
}

bool is_int(OBJ obj, int64 n) {
  return obj.core_data.int_ == n & obj.extra_data == INTEGER_MASK;
}

////////////////////////////////////////////////////////////////////////////////

bool is_inline_obj(OBJ obj) {
  assert((get_mem_layout(obj) == 0) == (get_physical_type(obj) <= MAX_INLINE_OBJ_TYPE_VALUE | obj.core_data.ptr == NULL));
  return get_mem_layout(obj) == 0;
}

bool is_ref_obj(OBJ obj) {
  return get_mem_layout(obj) != 0;
}

bool uses_try_mem(OBJ obj) {
  return get_mem_layout(obj) > 1;
}

bool is_gc_obj(OBJ obj) {
  return get_mem_layout(obj) == (is_in_try_state() ? 2 : 1);
}

////////////////////////////////////////////////////////////////////////////////

OBJ_TYPE get_ref_obj_type(OBJ obj) {
  assert(is_ref_obj(obj));

  OBJ_TYPE type = get_physical_type(obj);
  assert( type == TYPE_SEQUENCE | type == TYPE_SLICE | type == TYPE_SET | type == TYPE_BIN_REL |
          type == TYPE_LOG_MAP | type == TYPE_MAP | type == TYPE_TERN_REL | type == TYPE_TAG_OBJ);

  if (type == TYPE_SLICE)
    return TYPE_SEQUENCE;
  else if (type == TYPE_LOG_MAP)
    return TYPE_BIN_REL;
  else
    return type;
}

REF_OBJ *get_ref_obj_ptr(OBJ obj) {
  assert(is_ref_obj(obj));

  OBJ_TYPE type = get_physical_type(obj);
  assert( type == TYPE_SEQUENCE | type == TYPE_SLICE | type == TYPE_SET | type == TYPE_BIN_REL |
          type == TYPE_LOG_MAP | type == TYPE_MAP | type == TYPE_TERN_REL | type == TYPE_TAG_OBJ);

  if (type == TYPE_SLICE) {
    char *buffer_ptr = (char *) obj.core_data.ptr;
    return (REF_OBJ *) (buffer_ptr - (SEQ_BUFFER_FIELD_OFFSET + get_seq_offset(obj) * sizeof(OBJ)));
  }

  if (type == TYPE_SEQUENCE)
    return (REF_OBJ *) (((char *) obj.core_data.ptr) - SEQ_BUFFER_FIELD_OFFSET);

  return (REF_OBJ *) obj.core_data.ptr;
}

////////////////////////////////////////////////////////////////////////////////

bool are_shallow_eq(OBJ obj1, OBJ obj2) {
  return obj1.core_data.int_ == obj2.core_data.int_ && obj1.extra_data == obj2.extra_data;
}

int shallow_cmp(OBJ obj1, OBJ obj2) {
  assert(is_inline_obj(obj1) & is_inline_obj(obj2));

  uint64 extra_data_1 = obj1.extra_data;
  uint64 extra_data_2 = obj2.extra_data;

  if (extra_data_1 < extra_data_2)
    return 1;

  if (extra_data_1 > extra_data_2)
    return -1;

  int64 core_data_1 = obj1.core_data.int_;
  int64 core_data_2 = obj2.core_data.int_;

  if (core_data_1 < core_data_2)
    return 1;

  if (core_data_1 > core_data_2)
    return -1;

  return 0;
}
