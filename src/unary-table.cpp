#include "lib.h"


void unary_table_init(UNARY_TABLE *table) {
  const uint32 INIT_SIZE = 1024;
  uint64 *bitmap = (uint64 *) malloc(INIT_SIZE/8);
  memset(bitmap, 0, INIT_SIZE/8);
  table->bitmap = bitmap;
  table->size = INIT_SIZE;
  table->count = 0;
}

void unary_table_cleanup(UNARY_TABLE *table) {
  free(table->bitmap);
}

void unary_table_updates_init(UNARY_TABLE_UPDATES *table) {
  table->capacity = 0;
  table->deletes_count = 0;
  table->inserts_count = 0;
  table->buffer = NULL;
}

// Inserts and deletes are stored in the same buffer,
// deletes at the front and inserts at the back
void unary_table_updates_cleanup(UNARY_TABLE_UPDATES *table) {

}

////////////////////////////////////////////////////////////////////////////////

bool unary_table_contains(UNARY_TABLE *table, uint32 value) {
  assert(value < table->size);

  uint64 *bitmap = table->bitmap;
  uint32 idx = value >> 6;
  uint64 mask = 1ULL << (value % 64);
  return bitmap[idx] & mask;
}

////////////////////////////////////////////////////////////////////////////////

// Returns new capacity
uint32 unary_table_updates_resize(UNARY_TABLE_UPDATES *updates) {
  uint32 capacity = updates->capacity;
  uint32 new_capacity = capacity > 0 ? 2 * capacity : 32;
  uint32 *new_buffer = (uint32 *) malloc(new_capacity * sizeof(uint32));
  // uint32 *new_buffer = new_uint32_array(new_capacity);

  if (capacity > 0) {
    uint32 deletes_count = updates->deletes_count;
    uint32 inserts_count = updates->inserts_count;
    uint32 *buffer = updates->buffer;

    if (deletes_count > 0)
      memcpy(new_buffer, buffer, deletes_count * sizeof(uint32));
    if (inserts_count > 0) {
      uint32 *new_inserts = new_buffer + new_capacity - inserts_count;
      uint32 *inserts = buffer + capacity - inserts_count;
      memcpy(new_inserts, inserts, inserts_count * sizeof(uint32));
    }
    free(buffer);
    // delete_uint32_array(buffer, capacity);
  }

  updates->capacity = new_capacity;
  updates->buffer = new_buffer;
  return new_capacity;
}

void unary_table_insert(UNARY_TABLE_UPDATES *updates, uint32 value) {
  uint32 capacity = updates->capacity;
  uint32 deletes_count = updates->deletes_count;
  uint32 inserts_count = updates->inserts_count;

  if (deletes_count + inserts_count >= capacity)
    capacity = unary_table_updates_resize(updates);

  uint32 *next_slot = updates->buffer + capacity - 1 - inserts_count;
  *next_slot = value;
  updates->inserts_count = inserts_count + 1;
}

void unary_table_delete(UNARY_TABLE *, UNARY_TABLE_UPDATES *updates, uint32 value) {
  uint32 capacity = updates->capacity;
  uint32 deletes_count = updates->deletes_count;
  uint32 inserts_count = updates->inserts_count;

  if (deletes_count + inserts_count >= capacity)
    capacity = unary_table_updates_resize(updates);

  uint32 *next_slot = updates->buffer + deletes_count;
  *next_slot = value;
  updates->deletes_count = deletes_count + 1;
}

void unary_table_clear(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates) {
  uint64 *bitmap = table->bitmap;
  uint32 cell_count = table->size / 64;
  for (int i=0 ; i < cell_count ; i++) {
    uint64 cell = bitmap[i];
    for (int j=0 ; j < 64 ; j++)
      if ((cell >> j) & 1)
        unary_table_delete(table, updates, 64 * i + j);
  }
}

bool unary_table_updates_check(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates) {
  return true;
}

void unary_table_updates_apply(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates, VALUE_STORE *vs) {
  uint32 inserts_count = updates->inserts_count;
  uint32 deletes_count = updates->deletes_count;

  if (deletes_count > 0) {
    uint32 *deletes = updates->buffer;
    uint64 *bitmap = table->bitmap;
    for (uint32 i=0 ; i < deletes_count ; i++) {
      uint32 value = deletes[i];
      uint32 idx = value >> 6;
      uint64 mask = 1ULL << (value % 64);
      uint64 cell = bitmap[idx];
      if (cell & mask) {
        cell &= ~mask;
        bitmap[idx] = cell;
        table->count--;
      }
      else
        deletes[i] = 0xFFFFFFFFU;
    }
  }

  if (inserts_count > 0) {
    uint32 *inserts = updates->buffer + updates->capacity - inserts_count;
    uint32 max_val = *std::max_element(inserts, inserts + inserts_count);
    uint32 size = table->size;
    uint64 *bitmap = table->bitmap;
    if (max_val >= size) {
      // Reallocating the table
      uint32 new_size = 2 * size;
      while (max_val >= new_size)
        new_size *= 2;
      uint64 *bitmap = (uint64 *) realloc(bitmap, new_size / 8);
      memset(bitmap + (size / 64), 0, (new_size - size) / 8);
      size = new_size;
      table->size = size;
      table->bitmap = bitmap;
    }

    for (uint32 i=0 ; i < inserts_count ; i++) {
      uint32 value = inserts[i];
      uint32 idx = value >> 6;
      uint64 mask = 1ULL << (value % 64);
      uint64 cell = bitmap[idx];
      if (!(cell & mask)) {
        cell |= mask;
        bitmap[idx] = cell;
        table->count++;
        value_store_add_ref(vs, value);
      }
    }
  }
}

void unary_table_updates_finish(UNARY_TABLE_UPDATES *updates, VALUE_STORE *vs) {
  uint32 count = updates->deletes_count;
  uint32 *buffer = updates->buffer;
  if (count > 0) {
    for (uint32 i=0 ; i < count ; i++) {
      uint32 value = buffer[i];
      if (value != 0xFFFFFFFFU)
        value_store_release(vs, value);
    }
  }
  if (buffer != NULL)
    free(buffer);
  // // No need to delete anything for now, this memory is allocated in "temporary" memory, it is
  // // cleaned up automatically. An attempt to free it at this stage would actually cause a crash.
  // //## BUT WHY IS IT COUNTED AMONG THE LEAKED BLOCKS OF MEMORY?
  //   delete_uint32_array(buffer, updates->capacity);
}

////////////////////////////////////////////////////////////////////////////////

void unary_table_get_iter(UNARY_TABLE *table, UNARY_TABLE_ITER *iter) {
  if (table->count != 0) {
    uint64 *bitmap = table->bitmap;
    uint32 size = table->size;

    iter->bitmap = bitmap;
    iter->size = size;

    uint32 cell_count = size / 64;
    for (uint32 i=0 ; i < cell_count ; i++) {
      uint64 cell = bitmap[i];
      if (cell != 0)
        for (int j=0 ; j < 64 ; j++)
          if (((cell >> j) & 1) != 0) {
            iter->curr_value = 64 * i + j;
            return;
          }
    }
    internal_fail();
  }
  else {
    iter->bitmap = NULL;
    iter->size = 0;
    iter->curr_value = 0;
  }
}

uint32 unary_table_iter_get_field(UNARY_TABLE_ITER *iter) {
  assert(!unary_table_iter_is_out_of_range(iter));

  return iter->curr_value;
}

void unary_table_iter_next(UNARY_TABLE_ITER *iter) {
  assert(!unary_table_iter_is_out_of_range(iter));

  uint64 *bitmap = iter->bitmap;
  uint32 size = iter->size;
  uint32 curr_value = iter->curr_value;

  uint32 cell_count = size / 64;

  uint32 idx = curr_value / 64;
  uint32 offset = curr_value % 64 + 1;

  uint64 cell = bitmap[idx];
  if (cell >> offset != 0) {
    for (int i=offset ; i < 64 ; i++)
      if (((cell >> i) & 1) != 0) {
        iter->curr_value = 64 * idx + i;
        return;
      }
    internal_fail();
  }

  for (uint32 i=idx+1 ; i < cell_count ; i++) {
    cell = bitmap[i];
    if (cell != 0)
      for (int j=0 ; j < 64 ; j++)
        if (((cell >> j) & 1) != 0) {
          iter->curr_value = 64 * i + j;
          return;
        }
  }

  iter->bitmap = NULL;
  iter->size = 0;
  iter->curr_value = 0;
}

bool unary_table_iter_is_out_of_range(UNARY_TABLE_ITER *iter) {
  return iter->bitmap == NULL;
}

////////////////////////////////////////////////////////////////////////////////

OBJ copy_unary_table(UNARY_TABLE *table, VALUE_STORE *vs) {
  assert(table->size % 64 == 0);

  OBJ *slots = value_store_slot_array(vs);
  uint64 *bitmap = table->bitmap;
  uint32 size = table->size;
  uint32 count = table->count;

  if (count == 0)
    return make_empty_rel();

  SET_OBJ *set = new_set(count);
  OBJ *buffer = set->buffer;

  uint32 idx = 0;
  for (uint32 i=0 ; i < size/64 ; i++) {
    uint64 word = bitmap[i];
    for (int j=0 ; j < 64 ; j++)
      if ((word >> j) & 1) {
        OBJ obj = slots[64 * i + j];
        add_ref(obj);
        buffer[idx++] = obj;
      }
  }
  assert(idx == count);

  sort_obj_array(buffer, count);
  return make_set(set);
}

////////////////////////////////////////////////////////////////////////////////

void set_unary_table(UNARY_TABLE *table, UNARY_TABLE_UPDATES *updates, VALUE_STORE *vs, VALUE_STORE_UPDATES *vsu, OBJ set) {
  unary_table_clear(table, updates);

  if (is_empty_rel(set))
    return;

  SET_OBJ *ptr = get_set_ptr(set);
  uint32 size = ptr->size;
  OBJ *buffer = ptr->buffer;

  for (uint32 i=0 ; i < size ; i++) {
    OBJ obj = buffer[i];
    uint32 ref = lookup_value_ex(vs, vsu, obj);
    if (ref == -1) {
      add_ref(obj);
      ref = value_store_insert(vs, vsu, obj);
    }
    unary_table_insert(updates, ref);
  }
}
