#include "lib.h"


OBJ build_tern_rel(OBJ *vals1, OBJ *vals2, OBJ *vals3, uint32 size) {
  if (size == 0)
    return make_empty_rel();

  // Creating the array of indexes sorted by column 1, column 2, column 3, index
  uint32 *index = new_uint32_array(size);
  index_sort(index, vals1, vals2, vals3, size);

  // Counting the number of unique tuples and releasing unnecessary objects
  uint32 unique_tuples = 1;
  uint32 prev_idx = index[0];
  for (uint32 i=1 ; i < size ; i++) {
    uint32 idx = index[i];
    bool neq = comp_objs(vals1[idx], vals1[prev_idx]) != 0 ||
               comp_objs(vals2[idx], vals2[prev_idx]) != 0 ||
               comp_objs(vals3[idx], vals3[prev_idx]) != 0;
    if (neq) {
      unique_tuples++;
      prev_idx = idx;
    }
    else {
      // Duplicate tuple, marking the entry as duplicate and releasing the objects
      index[i] = INVALID_INDEX;
      release(vals1[idx]);
      release(vals2[idx]);
      release(vals3[idx]);
    }
  }

  // Creating the new binary relation object
  TERN_REL_OBJ *rel = new_tern_rel(unique_tuples);

  OBJ *col1 = get_col_array_ptr(rel, 0);
  OBJ *col2 = get_col_array_ptr(rel, 1);
  OBJ *col3 = get_col_array_ptr(rel, 2);

  // Copying the sorted, non-duplicate tuples into their final destination
  uint32 count = 0;
  for (uint32 i=0 ; i < size ; i++) {
    uint32 idx = index[i];
    if (idx != INVALID_INDEX) {
      col1[count] = vals1[idx];
      col2[count] = vals2[idx];
      col3[count] = vals3[idx];
      count++;
    }
  }
  assert(count == unique_tuples);

  // Creating the two indexes
  uint32 *index_1 = get_rotated_index(rel, 1);
  uint32 *index_2 = get_rotated_index(rel, 2);
  stable_index_sort(index_1, col2, col3, count);
  stable_index_sort(index_2, col3, count);

#ifndef NDEBUG
  for (uint32 i=1 ; i < count ; i++) {
    int cr_1 = comp_objs(col1[i-1], col1[i]);
    int cr_2 = comp_objs(col2[i-1], col2[i]);
    int cr_3 = comp_objs(col3[i-1], col3[i]);
    assert(cr_1 > 0 | (cr_1 == 0 & (cr_2 > 0 | (cr_2 == 0 & cr_3 > 0))));
  }

  for (uint32 i=1 ; i < count ; i++) {
    uint32 curr_idx = index_1[i];
    uint32 prev_idx = index_1[i-1];
    int cr_1 = comp_objs(col1[prev_idx], col1[curr_idx]);
    int cr_2 = comp_objs(col2[prev_idx], col2[curr_idx]);
    int cr_3 = comp_objs(col3[prev_idx], col3[curr_idx]);
    assert(cr_2 > 0 | (cr_2 == 0 & (cr_3 > 0 | (cr_3 == 0 & cr_1 > 0))));
  }

  for (uint32 i=1 ; i < count ; i++) {
    uint32 curr_idx = index_2[i];
    uint32 prev_idx = index_2[i-1];
    int cr_1 = comp_objs(col1[prev_idx], col1[curr_idx]);
    int cr_2 = comp_objs(col2[prev_idx], col2[curr_idx]);
    int cr_3 = comp_objs(col3[prev_idx], col3[curr_idx]);
    assert(cr_3 > 0 | (cr_3 == 0 & (cr_1 > 0 | (cr_1 == 0 & cr_2 > 0))));
  }
#endif

  delete_uint32_array(index, size);

  return make_tern_rel(rel);
}

////////////////////////////////////////////////////////////////////////////////

OBJ build_tern_rel(STREAM &stream1, STREAM &stream2, STREAM &stream3) {
  assert(stream1.count == stream2.count & stream2.count == stream3.count);

  if (stream1.count == 0)
    return make_empty_rel();

  OBJ rel = build_tern_rel(stream1.buffer, stream2.buffer, stream3.buffer, stream1.count);

  delete_obj_array(stream1.buffer, stream1.capacity);
  delete_obj_array(stream2.buffer, stream2.capacity);
  delete_obj_array(stream3.buffer, stream3.capacity);

  return rel;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void get_tern_rel_null_iter(TERN_REL_ITER &it) {
  it.col1 = NULL; // Not strictly necessary
  it.col2 = NULL; // Not strictly necessary
  it.col3 = NULL; // Not strictly necessary
  it.ordered_idxs = NULL;
  it.idx = 0;
  it.end = 0;
}

void get_tern_rel_iter(TERN_REL_ITER &it, OBJ rel) {
  assert(is_tern_rel(rel));

  if (is_ne_tern_rel(rel)) {
    TERN_REL_OBJ *ptr = get_tern_rel_ptr(rel);
    it.col1 = get_col_array_ptr(ptr, 0);
    it.col2 = get_col_array_ptr(ptr, 1);
    it.col3 = get_col_array_ptr(ptr, 2);
    it.ordered_idxs = NULL;
    it.idx = 0;
    it.end = ptr->size;
  }
  else
    get_tern_rel_null_iter(it);
}

void get_tern_rel_iter_by(TERN_REL_ITER &it, OBJ rel, int col_idx, OBJ arg) {
  assert(is_tern_rel(rel));
  assert(col_idx >= 0 & col_idx <= 2);

  if (is_ne_tern_rel(rel)) {
    TERN_REL_OBJ *ptr = get_tern_rel_ptr(rel);
    uint32 size = ptr->size;
    OBJ *col = get_col_array_ptr(ptr, col_idx);

    uint32 *index;
    uint32 count, first;
    if (col_idx == 0) {
      index = NULL;
      first = find_objs_range(col, size, arg, count);
    }
    else {
      index = get_rotated_index(ptr, col_idx);
      first = find_idxs_range(index, col, size, arg, count);
    }

    if (count > 0) {
      it.col1 = get_col_array_ptr(ptr, 0);
      it.col2 = get_col_array_ptr(ptr, 1);
      it.col3 = get_col_array_ptr(ptr, 2);
      it.ordered_idxs = index;
      it.idx = first;
      it.end = first + count;
      return;
    }
  }

  get_tern_rel_null_iter(it);
}

void get_tern_rel_iter_by(TERN_REL_ITER &it, OBJ rel, int major_col_idx, OBJ major_arg, OBJ minor_arg) {
  assert(is_tern_rel(rel));
  assert(major_col_idx >= 0 & major_col_idx <= 2);

  if (is_ne_tern_rel(rel)) {
    TERN_REL_OBJ *ptr = get_tern_rel_ptr(rel);
    uint32 size = ptr->size;
    OBJ *major_col = get_col_array_ptr(ptr, major_col_idx);
    OBJ *minor_col = get_col_array_ptr(ptr, (major_col_idx + 1) % 3);

    uint32 *index;
    uint32 count, first;
    if (major_col_idx == 0) {
      index = NULL;
      first = find_objs_range(major_col, minor_col, size, major_arg, minor_arg, count);
    }
    else {
      index = get_rotated_index(ptr, major_col_idx);
      first = find_idxs_range(index, major_col, minor_col, size, major_arg, minor_arg, count);
    }

    if (count > 0) {
      it.col1 = get_col_array_ptr(ptr, 0);
      it.col2 = get_col_array_ptr(ptr, 1);
      it.col3 = get_col_array_ptr(ptr, 2);
      it.ordered_idxs = index;
      it.idx = first;
      it.end = first + count;
      return;
    }
  }

  get_tern_rel_null_iter(it);
}
