#include "lib.h"


struct obj_less {
  bool operator () (OBJ obj1, OBJ obj2) {
    return comp_objs(obj1, obj2) > 0;
  }
};

struct obj_inline_less {
  bool operator () (OBJ obj1, OBJ obj2) {
    return shallow_cmp(obj1, obj2) > 0;
  }
};

////////////////////////////////////////////////////////////////////////////////

uint32 find_obj(OBJ *sorted_array, uint32 len, OBJ obj, bool &found) { // The array mustn't contain duplicates
  if (len > 0) {
    int64 low_idx = 0;
    int64 high_idx = len - 1;

    while (low_idx <= high_idx) {
      int64 middle_idx = (low_idx + high_idx) / 2;
      OBJ middle_obj = sorted_array[middle_idx];

      int cr = comp_objs(obj, middle_obj);

      if (cr == 0) {
        found = true;
        return middle_idx;
      }

      if (cr > 0)
        high_idx = middle_idx - 1;
      else
        low_idx = middle_idx + 1;
    }
  }

  found = false;
  return -1;
}

////////////////////////////////////////////////////////////////////////////////

uint32 count_at_start(uint32 *sorted_idx_array, OBJ *values, uint32 len, OBJ obj) {
  //## IMPLEMENT FOR REAL...
  int c = 0;
  while (c < len && comp_objs(obj, values[sorted_idx_array[c]]) == 0)
    c++;
  return c;
}

uint32 count_at_end(uint32 *sorted_idx_array, OBJ *values, uint32 len, OBJ obj) {
  //## IMPLEMENT FOR REAL...
  int c = 0;
  while (c < len && comp_objs(obj, values[sorted_idx_array[len-1-c]]) == 0)
    c++;
  return c;
}

uint32 find_idxs_range(uint32 *sorted_idx_array, OBJ *values, uint32 len, OBJ obj, uint32 &count) {
  int64 low_idx = 0;
  int64 high_idx = len - 1;

  while (low_idx <= high_idx) {
    int64 middle_idx = (low_idx + high_idx) / 2;
    OBJ middle_obj = values[sorted_idx_array[middle_idx]];

    int cr = comp_objs(obj, middle_obj);

    if (cr == 0) {
      int count_up = count_at_start(sorted_idx_array + middle_idx + 1, values, len - middle_idx - 1, obj);
      int count_down = count_at_end(sorted_idx_array, values, middle_idx, obj);
      count = 1 + count_up + count_down;
      return middle_idx - count_down;
    }

    if (cr > 0)
      high_idx = middle_idx - 1;
    else
      low_idx = middle_idx + 1;
  }

  count = 0;
  return INVALID_INDEX;
}

////////////////////////////////////////////////////////////////////////////////

uint32 count_at_start(OBJ *sorted_array, uint32 len, OBJ obj) {
  //## IMPLEMENT FOR REAL...
  int c = 0;
  while (c < len && comp_objs(obj, sorted_array[c]) == 0)
    c++;
  return c;
}

uint32 count_at_end(OBJ *sorted_array, uint32 len, OBJ obj) {
  //## IMPLEMENT FOR REAL...
  int c = 0;
  while (c < len && comp_objs(obj, sorted_array[len-1-c]) == 0)
    c++;
  return c;
}

uint32 find_objs_range(OBJ *sorted_array, uint32 len, OBJ obj, uint32 &count) {
  int64 low_idx = 0;
  int64 high_idx = len - 1;

  while (low_idx <= high_idx) {
    int64 middle_idx = (low_idx + high_idx) / 2;
    OBJ middle_obj = sorted_array[middle_idx];

    int cr = comp_objs(obj, middle_obj);

    if (cr == 0) {
      int count_up = count_at_start(sorted_array + middle_idx + 1, len - middle_idx - 1, obj);
      int count_down = count_at_end(sorted_array, middle_idx, obj);
      count = 1 + count_up + count_down;
      return middle_idx - count_down;
    }

    if (cr > 0)
      high_idx = middle_idx - 1;
    else
      low_idx = middle_idx + 1;
  }

  count = 0;
  return INVALID_INDEX;
}

////////////////////////////////////////////////////////////////////////////////

uint32 count_at_start(OBJ *major_col, OBJ *minor_col, uint32 len, OBJ major_arg, OBJ minor_arg) {
  //## IMPLEMENT FOR REAL...
  int c = 0;
  while (c < len && comp_objs(major_arg, major_col[c]) == 0 && comp_objs(minor_arg, minor_col[c]) == 0)
    c++;
  return c;
}

uint32 count_at_end(OBJ *major_col, OBJ *minor_col, uint32 len, OBJ major_arg, OBJ minor_arg) {
  //## IMPLEMENT FOR REAL...
  int c = 0;
  while (c < len && comp_objs(major_arg, major_col[len-1-c]) == 0 && comp_objs(minor_arg, minor_col[len-1-c]) == 0)
    c++;
  return c;
}

uint32 find_objs_range(OBJ *major_col, OBJ *minor_col, uint32 len, OBJ major_arg, OBJ minor_arg, uint32 &count) {
  int64 low_idx = 0;
  int64 high_idx = len - 1;

  while (low_idx <= high_idx) {
    int64 idx = (low_idx + high_idx) / 2;

    int cr = comp_objs(major_arg, major_col[idx]);
    if (cr == 0)
      cr = comp_objs(minor_arg, minor_col[idx]);

    if (cr == 0) {
      int count_up = count_at_start(major_col+idx+1, minor_col+idx+1, len-idx-1, major_arg, minor_arg);
      int count_down = count_at_end(major_col, minor_col, idx, major_arg, minor_arg);
      count = 1 + count_up + count_down;
      return idx - count_down;
    }

    if (cr > 0)
      high_idx = idx - 1;
    else
      low_idx = idx + 1;
  }

  count = 0;
  return INVALID_INDEX;
}

////////////////////////////////////////////////////////////////////////////////

uint32 count_at_start(uint32 *index, OBJ *major_col, OBJ *minor_col, uint32 len, OBJ major_arg, OBJ minor_arg) {
  //## IMPLEMENT FOR REAL...
  int c = 0;
  while (c < len) {
    uint32 idx = index[c];
    if (comp_objs(major_arg, major_col[idx]) == 0 && comp_objs(minor_arg, minor_col[idx]) == 0)
      c++;
    else
      break;
  }
  return c;
}

uint32 count_at_end(uint32 *index, OBJ *major_col, OBJ *minor_col, uint32 len, OBJ major_arg, OBJ minor_arg) {
  //## IMPLEMENT FOR REAL...
  int c = 0;
  while (c < len) {
    uint32 idx = index[len-c-1];
    if (comp_objs(major_arg, major_col[idx]) == 0 and comp_objs(minor_arg, minor_col[idx]) == 0)
      c++;
    else
      break;
  }
  return c;
}

uint32 find_idxs_range(uint32 *index, OBJ *major_col, OBJ *minor_col, uint32 len, OBJ major_arg, OBJ minor_arg, uint32 &count) {
  int64 low_idx = 0;
  int64 high_idx = len - 1;

  while (low_idx <= high_idx) {
    int64 idx = (low_idx + high_idx) / 2;
    uint32 dr_idx = index[idx];

    int cr = comp_objs(major_arg, major_col[dr_idx]);
    if (cr == 0)
      cr = comp_objs(minor_arg, minor_col[dr_idx]);

    if (cr == 0) {
      int count_up = count_at_start(index+idx+1, major_col, minor_col, len-idx-1, major_arg, minor_arg);
      int count_down = count_at_end(index, major_col, minor_col, idx, major_arg, minor_arg);
      count = 1 + count_up + count_down;
      return idx - count_down;
    }

    if (cr > 0)
      high_idx = idx - 1;
    else
      low_idx = idx + 1;
  }

  count = 0;
  return INVALID_INDEX;
}

////////////////////////////////////////////////////////////////////////////////

uint32 sort_and_release_dups(OBJ *objs, uint32 size) {
  if (size < 2)
    return size;

  uint32 low_idx = 0;
  uint32 high_idx = size - 1; // size is greater than 0 (actually 1) here, so this is always non-negative (actually positive)
  for ( ; ; ) {
    // Advancing the lower cursor to the next non-inline object
    while (low_idx < high_idx & is_inline_obj(objs[low_idx]))
      low_idx++;

    // Advancing the upper cursor to the next inline object
    while (high_idx > low_idx & not is_inline_obj(objs[high_idx]))
      high_idx--;

    if (low_idx == high_idx)
      break;

    OBJ tmp = objs[low_idx];
    objs[low_idx] = objs[high_idx];
    objs[high_idx] = tmp;
  }

  uint32 inline_count = is_inline_obj(objs[low_idx]) ? low_idx + 1 : low_idx;

  uint32 idx = 0;
  if (inline_count > 0) {
    std::sort(objs, objs+inline_count, obj_inline_less());

    OBJ last_obj = objs[0];
    for (uint32 i=1 ; i < inline_count ; i++) {
      OBJ next_obj = objs[i];
      if (!inline_eq(last_obj, next_obj)) {
        idx++;
        last_obj = next_obj;
        assert(idx <= i);
        if (idx != i)
          objs[idx] = next_obj;
      }
    }

    idx++;
    if (inline_count == size)
      return idx;
  }

  std::sort(objs+inline_count, objs+size, obj_less());

  if (idx != inline_count)
    objs[idx] = objs[inline_count];

  for (uint32 i=inline_count+1 ; i < size ; i++)
    // if (are_eq(objs[idx], objs[i]))
    if (comp_objs(objs[idx], objs[i]) == 0)
      release(objs[i]);
    else {
      idx++;
      assert(idx <= i);
      if (idx != i)
        objs[idx] = objs[i];
    }

  return idx + 1;
}

uint32 adjust_map_with_duplicate_keys(OBJ *keys, OBJ *values, uint32 size) {
  assert(size >= 2);

  OBJ prev_key = keys[0];
  OBJ prev_val = values[0];

  uint32 next_slot_idx = 1;
  uint32 i = 1;
  do {
    OBJ curr_key = keys[i];
    OBJ curr_val = values[i];

    if (comp_objs(curr_key, prev_key) == 0) {
      if (comp_objs(curr_val, prev_val) == 0) {
        release(curr_key);
        release(curr_val);
      }
      else
        soft_fail("Map contains duplicate keys");
    }
    else {
      keys[next_slot_idx] = curr_key;
      values[next_slot_idx] = curr_val;
      next_slot_idx++;
      prev_key = curr_key;
      prev_val = curr_val;
    }
  } while (++i < size);

  return next_slot_idx;
}

uint32 sort_and_check_no_dups(OBJ *keys, OBJ *values, uint32 size) {
  if (size < 2)
    return size;

  uint32 *idxs = new_uint32_array(size);
  index_sort(idxs, keys, size);

  for (uint32 i=0 ; i < size ; i++)
    if (idxs[i] != i) {
      OBJ key = keys[i];
      OBJ value = values[i];

      for (uint32 j = i ; ; ) {
        uint32 k = idxs[j];
        idxs[j] = j;

        if (k == i) {
          keys[j]   = key;
          values[j] = value;
          break;
        }
        else {
          keys[j]   = keys[k];
          values[j] = values[k];
          j = k;
        }
      }
    }

  delete_uint32_array(idxs, size);

  OBJ prev_key = keys[0];
  for (uint32 i=1 ; i < size ; i++) {
    OBJ curr_key = keys[i];
    if (comp_objs(curr_key, prev_key) == 0) {
      uint32 offset = i - 1;
      return offset + adjust_map_with_duplicate_keys(keys+offset, values+offset, size-offset);
    }
    prev_key = curr_key;
  }

  return size;
}


void sort_obj_array(OBJ *objs, uint32 len) {
  std::sort(objs, objs+len, obj_less());
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Returns:   > 0     if obj1 < obj2
//              0     if obj1 = obj2
//            < 0     if obj1 > obj2

int comp_objs(OBJ obj1, OBJ obj2) {
  if (are_shallow_eq(obj1, obj2))
    return 0;

  bool is_inline_1 = is_inline_obj(obj1);
  bool is_inline_2 = is_inline_obj(obj2);

  if (is_inline_1)
    if (is_inline_2)
      return shallow_cmp(obj1, obj2);
    else
      return 1;
  else if (is_inline_2)
    return -1;

  OBJ_TYPE type1 = get_logical_type(obj1);
  OBJ_TYPE type2 = get_logical_type(obj2);

  if (type1 != type2)
    return type2 - type1;

  uint32 count = 0;
  OBJ *elems1 = 0;
  OBJ *elems2 = 0;

  switch (type1) {
    case TYPE_SEQUENCE: {
      uint32 len1 = get_seq_length(obj1);
      uint32 len2 = get_seq_length(obj2);
      if (len1 != len2)
        return len2 - len1; //## BUG BUG BUG
      count = len1;
      elems1 = get_seq_buffer_ptr(obj1);
      elems2 = get_seq_buffer_ptr(obj2);
      break;
    }

    case TYPE_SET: {
      SET_OBJ *set1 = get_set_ptr(obj1);
      SET_OBJ *set2 = get_set_ptr(obj2);
      uint32 size1 = set1->size;
      uint32 size2 = set2->size;
      if (size1 != size2)
        return size2 - size1; //## BUG BUG BUG
      count = size1;
      elems1 = set1->buffer;
      elems2 = set2->buffer;
      break;
    }

    case TYPE_BIN_REL: {
      BIN_REL_OBJ *rel1 = get_bin_rel_ptr(obj1);
      BIN_REL_OBJ *rel2 = get_bin_rel_ptr(obj2);
      uint32 size1 = rel1->size;
      uint32 size2 = rel2->size;
      if (size1 != size2)
        return size2 - size1; //## BUG BUG BUG
      count = 2 * size1;
      elems1 = rel1->buffer;
      elems2 = rel2->buffer;
      break;
    }

    case TYPE_TERN_REL: {
      TERN_REL_OBJ *rel1 = get_tern_rel_ptr(obj1);
      TERN_REL_OBJ *rel2 = get_tern_rel_ptr(obj2);
      uint32 size1 = rel1->size;
      uint32 size2 = rel2->size;
      if (size1 != size2)
        return size2 - size1; //## BUG BUG BUG
      count = 3 * size1;
      elems1 = rel1->buffer;
      elems2 = rel2->buffer;
      break;
    }

    case TYPE_TAG_OBJ: {
      uint16 tag_idx_1 = get_tag_idx(obj1);
      uint16 tag_idx_2 = get_tag_idx(obj2);
      if (tag_idx_1 != tag_idx_2)
        return tag_idx_2 - tag_idx_1;
      return comp_objs(get_inner_obj(obj1), get_inner_obj(obj2));
    }

    default:
      internal_fail();
  }

  for (uint32 i=0 ; i < count ; i++) {
    int cr = comp_objs(elems1[i], elems2[i]);
    if (cr != 0)
      return cr;
  }

  return 0;
}