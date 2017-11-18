#include "lib.h"


bool inline_eq(OBJ obj1, OBJ obj2) {
  // assert(is_inline_obj(obj2) & !is_float(obj2));
  assert(is_inline_obj(obj2));
  return are_shallow_eq(obj1, obj2);
}

bool are_eq(OBJ obj1, OBJ obj2) {
  return comp_objs(obj1, obj2) == 0;
}

bool is_out_of_range(SET_ITER &it) {
  return it.idx >= it.size;
}

bool is_out_of_range(SEQ_ITER &it) {
  return it.idx >= it.len;
}

bool is_out_of_range(BIN_REL_ITER &it) {
  return it.idx >= it.end;
}

bool is_out_of_range(TERN_REL_ITER &it) {
  return it.idx >= it.end;
}

bool has_elem(OBJ set, OBJ elem) {
  if (is_empty_rel(set))
    return false;
  SET_OBJ *s = get_set_ptr(set);
  bool found;
  find_obj(s->buffer, s->size, elem, found);
  return found;
}

bool has_pair(OBJ rel, OBJ arg0, OBJ arg1) {
  if (is_empty_rel(rel))
    return false;

  BIN_REL_OBJ *ptr = get_bin_rel_ptr(rel);
  uint32 size = ptr->size;
  OBJ *left_col = get_left_col_array_ptr(ptr);
  OBJ *right_col = get_right_col_array_ptr(ptr);

  if (is_ne_map(rel)) {
    bool found;
    uint32 idx = find_obj(left_col, size, arg0, found);
    if (!found)
      return false;
    return comp_objs(right_col[idx], arg1) == 0;
  }

  uint32 count;
  uint32 idx = find_objs_range(left_col, size, arg0, count);
  if (count == 0)
    return false;
  bool found;
  find_obj(right_col+idx, count, arg1, found);
  return found;
}

bool has_key(OBJ rel, OBJ arg1) {
  if (is_empty_rel(rel))
    return false;

  BIN_REL_OBJ *ptr = get_bin_rel_ptr(rel);
  uint32 size = ptr->size;
  OBJ *left_col = get_left_col_array_ptr(ptr);

  if (is_ne_map(rel)) {
    bool found;
    uint32 idx = find_obj(left_col, size, arg1, found);
    return found;
  }

  uint32 count;
  uint32 idx = find_objs_range(left_col, size, arg1, count);
  return count > 0;
}

bool has_field(OBJ rec_or_tag_rec, uint16 field_symb_idx) {
  OBJ rec = is_tag_obj(rec_or_tag_rec) ? get_inner_obj(rec_or_tag_rec) : rec_or_tag_rec;

  if (!is_empty_rel(rec)) {
    BIN_REL_OBJ *ptr = get_bin_rel_ptr(rec);
    uint32 size = ptr->size;
    OBJ *keys = ptr->buffer;
    for (uint32 i=0 ; i < size ; i++)
      if (is_symb(keys[i], field_symb_idx))
        return true;
  }

  return false;
}

bool has_triple(OBJ rel, OBJ arg1, OBJ arg2, OBJ arg3) {
  assert(is_tern_rel(rel));

  if (is_empty_rel(rel))
    return false;

  TERN_REL_OBJ *ptr = get_tern_rel_ptr(rel);
  uint32 size = ptr->size;
  OBJ *col1 = get_col_array_ptr(ptr, 0);

  uint32 count;
  uint32 first = find_objs_range(col1, size, arg1, count);
  if (count == 0)
    return false;

  OBJ *col2 = get_col_array_ptr(ptr, 1);

  first = first + find_objs_range(col2+first, count, arg2, count);
  if (count == 0)
    return false;

  OBJ *col3 = get_col_array_ptr(ptr, 2);

  bool found;
  find_obj(col3+first, count, arg3, found);
  return found;
}

////////////////////////////////////////////////////////////////////////////////

int64 get_int_val(OBJ obj) {
  assert(is_int(obj));

  return get_int(obj);
}

uint32 get_size(OBJ coll) {
  assert(is_seq(coll) | is_set(coll) | is_bin_rel(coll) | is_tern_rel(coll));

  if (is_seq(coll))
    return get_seq_length(coll);

  if (is_empty_rel(coll))
    return 0;

  if (is_ne_set(coll))
    return get_set_ptr(coll)->size;

  if (is_ne_bin_rel(coll))
    return get_bin_rel_ptr(coll)->size;

  return get_tern_rel_ptr(coll)->size;
}

int64 float_bits(OBJ obj) {
  double x = get_float(obj);
  return *((int64 *) &x);
}

int64 mantissa(OBJ obj) {
  int64 mantissa;
  int32 dec_exp;
  mantissa_and_dec_exp(get_float(obj), mantissa, dec_exp);
  return mantissa;
}

int64 dec_exp(OBJ obj) {
  int64 mantissa;
  int32 dec_exp;
  mantissa_and_dec_exp(get_float(obj), mantissa, dec_exp);
  return dec_exp;
}

int64 rand_nat(int64 max) {
  assert(max > 0);
  return rand() % max; //## BUG: THE FUNCTION rand() ONLY GENERATES A LIMITED RANGE OF INTEGERS
}

int64 unique_nat() {
  static int64 next_val = 0;
  return next_val++;
}

////////////////////////////////////////////////////////////////////////////////

OBJ obj_neg(OBJ obj) {
  assert(is_bool(obj));
  return make_bool(!get_bool(obj));
}

OBJ at(OBJ seq, int64 idx) {
  assert(is_seq(seq));
  if (((uint64) idx) >= get_seq_length(seq))
    soft_fail("Invalid sequence index");
  return get_seq_buffer_ptr(seq)[idx];
}

OBJ get_tag(OBJ obj) {
  return make_symb(get_tag_idx(obj));
}

OBJ get_curr_obj(SEQ_ITER &it) {
  assert(!is_out_of_range(it));
  return it.buffer[it.idx];
}

OBJ get_curr_obj(SET_ITER &it) {
  assert(!is_out_of_range(it));
  return it.buffer[it.idx];
}

OBJ get_curr_left_arg(BIN_REL_ITER &it) {
  assert(!is_out_of_range(it));
  uint32 idx = it.rev_idxs != NULL ? it.rev_idxs[it.idx] : it.idx;
  return it.left_col[idx];
}

OBJ get_curr_right_arg(BIN_REL_ITER &it) {
  assert(!is_out_of_range(it));
  uint32 idx = it.rev_idxs != NULL ? it.rev_idxs[it.idx] : it.idx;
  return it.right_col[idx];
}

OBJ tern_rel_it_get_left_arg(TERN_REL_ITER &it) {
  assert(!is_out_of_range(it));
  uint32 idx = it.ordered_idxs != NULL ? it.ordered_idxs[it.idx] : it.idx;
  return it.col1[idx];
}

OBJ tern_rel_it_get_mid_arg(TERN_REL_ITER &it) {
  assert(!is_out_of_range(it));
  uint32 idx = it.ordered_idxs != NULL ? it.ordered_idxs[it.idx] : it.idx;
  return it.col2[idx];
}

OBJ tern_rel_it_get_right_arg(TERN_REL_ITER &it) {
  assert(!is_out_of_range(it));
  uint32 idx = it.ordered_idxs != NULL ? it.ordered_idxs[it.idx] : it.idx;
  return it.col3[idx];
}

OBJ rand_set_elem(OBJ set) {
  SET_OBJ *set_ptr = get_set_ptr(set);
  uint32 idx = rand() % set_ptr->size;
  return set_ptr->buffer[idx];
}

OBJ lookup(OBJ rel, OBJ key) {
  if (!is_empty_rel(rel)) {
    BIN_REL_OBJ *ptr = get_bin_rel_ptr(rel);
    uint32 size = ptr->size;
    OBJ *keys = ptr->buffer;
    OBJ *values = keys + size;
    OBJ_TYPE rel_type = get_physical_type(rel);
    if (rel_type == TYPE_MAP | rel_type == TYPE_LOG_MAP) {
      bool found;
      uint32 idx = find_obj(keys, size, key, found);
      if (found)
        return values[idx];
    }
    else {
      assert(rel_type == TYPE_BIN_REL);
      uint32 count;
      uint32 idx = find_objs_range(keys, size, key, count);
      if (count == 1)
        return values[idx];
      if (count > 1)
        soft_fail("Key is not unique. Lookup failed");
    }
  }

  if (is_empty_rel(rel))
    soft_fail("Map is empty. Lookup failed");

  if (is_symb(key)) {
    char buff[1024];
    strcpy(buff, "Map key not found: ");
    uint32 len = strlen(buff);
    printed_obj(key, buff+len, sizeof(buff)-len-1);
    soft_fail(buff);
  }

  soft_fail("Map key not found");
}

OBJ lookup_field(OBJ rec_or_tag_rec, uint16 field_symb_idx) {
  OBJ rec = is_tag_obj(rec_or_tag_rec) ? get_inner_obj(rec_or_tag_rec) : rec_or_tag_rec;

  if (!is_empty_rel(rec)) {
    BIN_REL_OBJ *ptr = get_bin_rel_ptr(rec);
    uint32 size = ptr->size;
    OBJ *keys = ptr->buffer;
    OBJ *values = keys + size;
    for (uint32 i=0 ; i < size ; i++)
      if (is_symb(keys[i], field_symb_idx))
        return values[i];
  }

  internal_fail();
}
