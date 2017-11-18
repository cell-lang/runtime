#include "lib.h"


struct obj_idx_less {
  OBJ *objs;
  obj_idx_less(OBJ *objs) : objs(objs) {}

  bool operator () (uint32 idx1, uint32 idx2) {
    return comp_objs(objs[idx1], objs[idx2]) > 0;
  }
};

struct obj_idx_less_no_eq {
  OBJ *values;
  obj_idx_less_no_eq(OBJ *values) : values(values) {}

  bool operator () (uint32 idx1, uint32 idx2) {
    int cr = comp_objs(values[idx1], values[idx2]);
    return cr != 0 ? cr > 0 : idx1 < idx2;
  }
};

////////////////////////////////////////////////////////////////////////////////

struct obj_pair_idx_less {
  OBJ *major_sort, *minor_sort;
  obj_pair_idx_less(OBJ *major_sort, OBJ *minor_sort) : major_sort(major_sort), minor_sort(minor_sort) {}

  bool operator () (uint32 idx1, uint32 idx2) {
    int cr = comp_objs(major_sort[idx1], major_sort[idx2]);
    if (cr != 0)
      return cr > 0;
    cr = comp_objs(minor_sort[idx1], minor_sort[idx2]);
    if (cr != 0)
      return cr > 0;
    return idx1 < idx2;
  }
};

////////////////////////////////////////////////////////////////////////////////

struct obj_triple_idx_less {
  OBJ *col1, *col2, *col3;
  obj_triple_idx_less(OBJ *col1, OBJ *col2, OBJ *col3) : col1(col1), col2(col2), col3(col3) {}

  bool operator () (uint32 idx1, uint32 idx2) {
    int cr = comp_objs(col1[idx1], col1[idx2]);
    if (cr != 0)
      return cr > 0;
    cr = comp_objs(col2[idx1], col2[idx2]);
    if (cr != 0)
      return cr > 0;
    cr = comp_objs(col3[idx1], col3[idx2]);
    if (cr != 0)
      return cr > 0;
    return idx1 < idx2;
  }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void stable_index_sort(uint32 *index, OBJ *values, uint32 count) {
  for (uint32 i=0 ; i < count ; i++)
    index[i] = i;
  std::sort(index, index+count, obj_idx_less_no_eq(values));
}

void stable_index_sort(uint32 *index, OBJ *major_sort, OBJ *minor_sort, uint32 count) {
  for (uint32 i=0 ; i < count ; i++)
    index[i] = i;
  std::sort(index, index+count, obj_pair_idx_less(major_sort, minor_sort));
}

void stable_index_sort(uint32 *index, OBJ *major_sort, OBJ *middle_sort, OBJ *minor_sort, uint32 count) {
  for (uint32 i=0 ; i < count ; i++)
    index[i] = i;
  std::sort(index, index+count, obj_triple_idx_less(major_sort, middle_sort, minor_sort));
}

////////////////////////////////////////////////////////////////////////////////

void index_sort(uint32 *index, OBJ *values, uint32 count) {
  for (uint32 i=0 ; i < count ; i++)
    index[i] = i;
  std::sort(index, index+count, obj_idx_less(values));
}

void index_sort(uint32 *index, OBJ *major_sort, OBJ *minor_sort, uint32 count) {
  stable_index_sort(index, major_sort, minor_sort, count);
}

void index_sort(uint32 *index, OBJ *major_sort, OBJ *middle_sort, OBJ *minor_sort, uint32 count) {
  stable_index_sort(index, major_sort, middle_sort, minor_sort, count);
}
