#include "lib.h"
#include "table-utils.h"


void binary_table_init(BINARY_TABLE *table) {

}

void binary_table_cleanup(BINARY_TABLE *table) {

}

void binary_table_updates_init(BINARY_TABLE_UPDATES *updates) {

}

void binary_table_updates_cleanup(BINARY_TABLE_UPDATES *updates) {

}

////////////////////////////////////////////////////////////////////////////////

bool binary_table_contains(BINARY_TABLE *table, uint32 left_val, uint32 right_val) {
  std::set<uint64> &left_to_right = table->left_to_right;
  std::set<uint64>::iterator it = left_to_right.find(pack(left_val, right_val));
  return it != left_to_right.end();
}

////////////////////////////////////////////////////////////////////////////////

void binary_table_delete_range_(BINARY_TABLE_ITER *iter, BINARY_TABLE_UPDATES *updates) {
  bool reversed = iter->reversed;
  std::vector<uint64> &deletes = updates->deletes;
  while (!binary_table_iter_is_out_of_range(iter)) {
    uint64 pair = *iter->iter;
    deletes.push_back(reversed ? swap(pair) : pair);
    binary_table_iter_next(iter);
  }
}

void binary_table_delete(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, uint32 left_val, uint32 right_val) {
  if (binary_table_contains(table, left_val, right_val))
    updates->deletes.push_back(pack(left_val, right_val));
}

void binary_table_delete_by_col_0(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, uint32 value) {
  BINARY_TABLE_ITER iter;
  binary_table_get_iter_by_col_0(table, &iter, value);
  binary_table_delete_range_(&iter, updates);
}

void binary_table_delete_by_col_1(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, uint32 value) {
  BINARY_TABLE_ITER iter;
  binary_table_get_iter_by_col_1(table, &iter, value);
  binary_table_delete_range_(&iter, updates);
}

void binary_table_clear(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates) {
  BINARY_TABLE_ITER iter;
  binary_table_get_iter(table, &iter);
  binary_table_delete_range_(&iter, updates);
}

void binary_table_insert(BINARY_TABLE_UPDATES *updates, uint32 left_val, uint32 right_val) {
  updates->inserts.push_back(pack(left_val, right_val));
}

void binary_table_updates_apply(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, VALUE_STORE *vs0, VALUE_STORE *vs1) {
  std::set<uint64> &left_to_right = table->left_to_right;
  std::set<uint64> &right_to_left = table->right_to_left;

  if (!updates->deletes.empty()) {
    uint32 count = updates->deletes.size();
    uint64 *deletes = &updates->deletes.front();
    for (uint32 i=0 ; i < count ; i++) {
      uint64 pair = deletes[i];
      if (left_to_right.erase(pair) > 0)
        right_to_left.erase(swap(pair));
      else
        deletes[i] = 0xFFFFFFFFFFFFFFFFULL;
    }
  }

  if (!updates->inserts.empty()) {
    uint32 count = updates->inserts.size();
    uint64 *inserts = &updates->inserts.front();
    for (uint32 i=0 ; i < count ; i++) {
      uint64 pair = inserts[i];
      if (left_to_right.insert(pair).second) {
        right_to_left.insert(swap(pair));
        value_store_add_ref(vs0, left(pair));
        value_store_add_ref(vs1, right(pair));
      }
    }
  }
}

void binary_table_updates_finish(BINARY_TABLE_UPDATES *updates, VALUE_STORE *vs0, VALUE_STORE *vs1) {
  if (!updates->deletes.empty()) {
    uint32 count = updates->deletes.size();
    uint64 *deletes = &updates->deletes.front();
    for (uint32 i=0 ; i < count ; i++) {
      uint64 pair = deletes[i];
      if (pair != 0xFFFFFFFFFFFFFFFFULL) {
        value_store_release(vs0, left(pair));
        value_store_release(vs1, right(pair));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void binary_table_get_iter_by_col_0(BINARY_TABLE *table, BINARY_TABLE_ITER *iter, uint32 value) {
  std::set<uint64> &left_to_right = table->left_to_right;
  iter->iter = left_to_right.lower_bound(pack(value, 0));
  iter->end = left_to_right.end();
  iter->value = value;
  iter->reversed = false;
}

void binary_table_get_iter_by_col_1(BINARY_TABLE *table, BINARY_TABLE_ITER *iter, uint32 value) {
  std::set<uint64> &right_to_left = table->right_to_left;
  iter->iter = right_to_left.lower_bound(pack(value, 0));
  iter->end = right_to_left.end();
  iter->value = value;
  iter->reversed = true;
}

void binary_table_get_iter(BINARY_TABLE *table, BINARY_TABLE_ITER *iter) {
  std::set<uint64> &left_to_right = table->left_to_right;
  iter->iter = left_to_right.begin();
  iter->end = left_to_right.end();
  iter->value = 0xFFFFFFFFU;
  iter->reversed = false;
}

////////////////////////////////////////////////////////////////////////////////

bool binary_table_iter_is_out_of_range(BINARY_TABLE_ITER *iter) {
  std::set<uint64>::iterator it = iter->iter;
  return it == iter->end || (*it >> 32) > iter->value;
}

uint32 binary_table_iter_get_left_field(BINARY_TABLE_ITER *iter) {
  return *iter->iter >> (iter->reversed ? 0 : 32);
}

uint32 binary_table_iter_get_right_field(BINARY_TABLE_ITER *iter) {
  return *iter->iter >> (iter->reversed ? 32 : 0);
}

void binary_table_iter_next(BINARY_TABLE_ITER *iter) {
  assert(!binary_table_iter_is_out_of_range(iter));
  iter->iter++;
}

////////////////////////////////////////////////////////////////////////////////

bool binary_table_updates_check_0(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates) {
  sort_unique(updates->inserts);
  return table_updates_check_key<col_0>(updates->inserts, updates->deletes, table->left_to_right);
}

bool binary_table_updates_check_1(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates) {
  sort_unique(updates->inserts);
  return table_updates_check_key<col_1>(updates->inserts, updates->deletes, table->right_to_left);
}

bool binary_table_updates_check_0_1(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates) {
  return binary_table_updates_check_0(table, updates) &&
    table_updates_check_key<col_1>(updates->inserts, updates->deletes, table->right_to_left);
}

////////////////////////////////////////////////////////////////////////////////

OBJ copy_binary_table(BINARY_TABLE *table, VALUE_STORE *vs1, VALUE_STORE *vs2, bool flip_cols) {
  OBJ *slots1 = value_store_slot_array(vs1);
  OBJ *slots2 = value_store_slot_array(vs2);

  std::set<uint64> &rows = table->left_to_right;
  uint32 size = rows.size();

  if (size == 0)
    return make_empty_rel();

  OBJ *col1 = new_obj_array(2 * size);
  OBJ *col2 = col1 + size;

  uint32 idx = 0;
  for (std::set<uint64>::iterator it=rows.begin(); it != rows.end() ; it++) {
    uint64 row = *it;
    col1[idx] = slots1[left(row)];
    col2[idx++] = slots2[right(row)];
  }
  assert(idx == size);

  for (int64 i=0 ; i < 2 * size ; i++)
    add_ref(col1[i]);

  OBJ rel = build_bin_rel(flip_cols ? col2 : col1, flip_cols ? col1 : col2, size);

  delete_obj_array(col1, 2 * size);

  return rel;
}

////////////////////////////////////////////////////////////////////////////////

void set_binary_table(BINARY_TABLE *table, BINARY_TABLE_UPDATES *updates, VALUE_STORE *vs1, VALUE_STORE *vs2,
  VALUE_STORE_UPDATES *vsu1, VALUE_STORE_UPDATES *vsu2, OBJ rel, bool flip_cols) {
  binary_table_clear(table, updates);

  if (is_empty_rel(rel))
    return;

  BIN_REL_OBJ *ptr = get_bin_rel_ptr(rel);
  uint32 size = ptr->size;
  OBJ *col1 = flip_cols ? get_right_col_array_ptr(ptr) : get_left_col_array_ptr(ptr);
  OBJ *col2 = flip_cols ? get_left_col_array_ptr(ptr) : get_right_col_array_ptr(ptr);

  for (uint32 i=0 ; i < size ; i++) {
    OBJ obj = col1[i];
    uint32 ref1 = lookup_value_ex(vs1, vsu1, obj);
    if (ref1 == -1) {
      add_ref(obj);
      ref1 = value_store_insert(vs1, vsu1, obj);
    }

    obj = col2[i];
    uint32 ref2 = lookup_value_ex(vs2, vsu2, obj);
    if (ref2 == -1) {
      add_ref(obj);
      ref2 = value_store_insert(vs2, vsu2, obj);
    }

    binary_table_insert(updates, ref1, ref2);
  }
}
