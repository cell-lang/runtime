#include "lib.h"


const uint32 BASE_VALUE   = 17;
const uint32 MULTIPLIER   = 37;

const uint32 MULT_BASE_VALUE = BASE_VALUE * MULTIPLIER;

////////////////////////////////////////////////////////////////////////////////

uint32 combined_hash_code(uint32 start_value, OBJ *array, uint32 count) {
  uint32 hash_code = start_value;
  for (uint32 i=0 ; i < count ; i++)
    hash_code = MULTIPLIER * hash_code + compute_hash_code(array[i]);
  return hash_code;
}

uint32 compute_hash_code(OBJ obj) {
  if (is_tag_obj(obj))
    return MULTIPLIER * (MULT_BASE_VALUE + get_tag_idx(obj)) + compute_hash_code(get_inner_obj(obj));

  switch (get_physical_type(obj)) {
    case TYPE_BLANK_OBJ:
    case TYPE_NULL_OBJ:
      fail();

    case TYPE_SYMBOL:
      return MULT_BASE_VALUE + get_symb_idx(obj);

    case TYPE_INTEGER:
    case TYPE_FLOAT: {
      uint64 core_data = obj.core_data.int_;
      return MULT_BASE_VALUE + (uint32) (core_data ^ (core_data >> 32));
    }

    case TYPE_SEQUENCE: {
      uint32 size = get_seq_length(obj);
      return combined_hash_code(MULT_BASE_VALUE + size, size > 0 ? get_seq_buffer_ptr(obj) : NULL, size);
    }

    case TYPE_SET: {
      if (is_empty_rel(obj))
        return MULT_BASE_VALUE;
      SET_OBJ *ptr = get_set_ptr(obj);
      uint32 size = ptr->size;
      return combined_hash_code(MULT_BASE_VALUE + size, ptr->buffer, size);
    }

    case TYPE_BIN_REL: {
      BIN_REL_OBJ *ptr = get_bin_rel_ptr(obj);
      uint32 size = ptr->size;
      return combined_hash_code(MULT_BASE_VALUE + size, ptr->buffer, 2 * size);
    }

    case TYPE_TERN_REL: {
      TERN_REL_OBJ *ptr = get_tern_rel_ptr(obj);
      uint32 size = ptr->size;
      return combined_hash_code(MULT_BASE_VALUE + size, ptr->buffer, 3 * size);
    }

    case TYPE_TAG_OBJ:
      fail();

    case TYPE_SLICE: {
      uint32 size = get_seq_length(obj);
      return combined_hash_code(MULT_BASE_VALUE + size, get_seq_buffer_ptr(obj), size);
    }

    case TYPE_MAP:
    case TYPE_LOG_MAP: {
      BIN_REL_OBJ *ptr = get_bin_rel_ptr(obj);
      uint32 size = ptr->size;
      return combined_hash_code(MULT_BASE_VALUE + size, ptr->buffer, 2 * size);
    }
  }
  fail();
}
