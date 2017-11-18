#include "lib.h"


uint64 set_obj_mem_size(uint64 size) {
  assert(size > 0);
  return sizeof(SET_OBJ) + (size - 1) * sizeof(OBJ);
}

uint64 seq_obj_mem_size(uint64 capacity) {
  assert(capacity > 0);
  return sizeof(SEQ_OBJ) + (capacity - 1) * sizeof(OBJ);
}

uint64 bin_rel_obj_mem_size(uint64 size) {
  assert(size > 0);
  return sizeof(BIN_REL_OBJ) + (2 * size - 1) * sizeof(OBJ) + size * sizeof(uint32);
}

uint32 tern_rel_obj_mem_size(uint64 size) {
  assert(size > 0);
  return sizeof(TERN_REL_OBJ) + (3 * size - 1) * sizeof(OBJ) + 2 * size * sizeof(uint32);
}

uint64 map_obj_mem_size(uint64 size) {
  assert(size > 0);
  return bin_rel_obj_mem_size(size);
}

uint64 tag_obj_mem_size() {
  return sizeof(TAG_OBJ);
}

////////////////////////////////////////////////////////////////////////////////

OBJ *get_left_col_array_ptr(BIN_REL_OBJ *rel) {
  return rel->buffer;
}

OBJ *get_right_col_array_ptr(BIN_REL_OBJ *rel) {
  return rel->buffer + rel->size;
}

uint32 *get_right_to_left_indexes(BIN_REL_OBJ *rel) {
  return (uint32 *) (rel->buffer + 2 * rel->size);
}

////////////////////////////////////////////////////////////////////////////////

OBJ *get_col_array_ptr(TERN_REL_OBJ *rel, int idx) {
  assert(idx >= 0 & idx <= 2);
  return rel->buffer + idx * rel->size;
}

uint32 *get_rotated_index(TERN_REL_OBJ *rel, int amount) {
  assert(amount == 1 | amount == 2);
  uint32 size = rel->size;
  uint32 *base_ptr = (uint32 *) (rel->buffer + 3 * size);
  return base_ptr + (amount-1) * size;
}

////////////////////////////////////////////////////////////////////////////////

uint32 seq_capacity(uint64 byte_size) {
  return (byte_size - sizeof(SEQ_OBJ)) / sizeof(OBJ) + 1;
}

////////////////////////////////////////////////////////////////////////////////

SEQ_OBJ *new_seq(uint32 length) {
  assert(length > 0);

  if (length > 0xFFFFFFF)
    impl_fail("Maximum permitted sequence length (2^28-1) exceeded");

  uint32 actual_byte_size;
  SEQ_OBJ *seq = (SEQ_OBJ *) new_obj(seq_obj_mem_size(length), actual_byte_size);
  seq->ref_obj.ref_count = 1;
  seq->capacity = seq_capacity(actual_byte_size);
  seq->size = length;
  return seq;
}

SET_OBJ *new_set(uint32 size) {
  SET_OBJ *set = (SET_OBJ *) new_obj(set_obj_mem_size(size));
  set->ref_obj.ref_count = 1;
  set->size = size;
  return set;
}

BIN_REL_OBJ *new_map(uint32 size) {
  assert(size > 0);

  BIN_REL_OBJ *map = (BIN_REL_OBJ *) new_obj(map_obj_mem_size(size));
  map->ref_obj.ref_count = 1;
  map->size = size;
  uint32 *rev_idxs = get_right_to_left_indexes(map);
  rev_idxs[0] = INVALID_INDEX;
  return map;
}

BIN_REL_OBJ *new_bin_rel(uint32 size) {
  assert(size > 0);

  BIN_REL_OBJ *rel = (BIN_REL_OBJ *) new_obj(bin_rel_obj_mem_size(size));
  rel->ref_obj.ref_count = 1;
  rel->size = size;
  return rel;
}

TERN_REL_OBJ *new_tern_rel(uint32 size) {
  assert(size > 0);

  TERN_REL_OBJ *rel = (TERN_REL_OBJ *) new_obj(tern_rel_obj_mem_size(size));
  rel->ref_obj.ref_count = 1;
  rel->size = size;
  return rel;
}

TAG_OBJ *new_tag_obj() {
  TAG_OBJ *tag_obj = (TAG_OBJ *) new_obj(tag_obj_mem_size());
  tag_obj->ref_obj.ref_count = 1;
  tag_obj->unused_field = 0;
  return tag_obj;
}

////////////////////////////////////////////////////////////////////////////////

//## WHY ISN'T THERE A shrink_map()?

SET_OBJ *shrink_set(SET_OBJ *set, uint32 new_size) {
  assert(new_size < set->size);
  assert(set->ref_obj.ref_count == 1);

  uint32 size = set->size;
  uint32 mem_size = set_obj_mem_size(size);
  uint32 new_mem_size = set_obj_mem_size(new_size);

  if (mem_size == new_mem_size) {
    // If the memory footprint is exactly the same, I can safely reuse
    // the same object without causing problems in the memory allocator.
    set->size = new_size;
    return set;
  }

  SET_OBJ *new_set = ::new_set(new_size);
  memcpy(new_set->buffer, set->buffer, new_size * sizeof(OBJ));
  free_obj(set, mem_size);

  return new_set;
}

////////////////////////////////////////////////////////////////////////////////

OBJ *new_obj_array(uint32 size) {
  return (OBJ *) new_obj(size * sizeof(OBJ));
}

void delete_obj_array(OBJ *buffer, uint32 size) {
  free_obj(buffer, size * sizeof(OBJ));
}

OBJ* resize_obj_array(OBJ* buffer, uint32 size, uint32 new_size) {
  return (OBJ *) resize_obj(buffer, size * sizeof(OBJ), new_size * sizeof(OBJ));
}

uint32 *new_uint32_array(uint32 size) {
  return (uint32 *) new_obj(size * sizeof(uint32));
}

void delete_uint32_array(uint32 *buffer, uint32 size) {
  free_obj(buffer, size * sizeof(uint32));
}

int32 *new_int32_array(uint32 size) {
  return (int32 *) new_obj(size * sizeof(int32));
}

void delete_int32_array(int32 *buffer, uint32 size) {
  free_obj(buffer, size * sizeof(int32));
}

char *new_byte_array(uint32 size) {
  return (char *) new_obj(size);
}

void delete_byte_array(char *buffer, uint32 size) {
  free_obj(buffer, size);
}

void **new_ptr_array(uint32 size) {
  return (void **) new_obj(size * sizeof(void *));
}

void delete_ptr_array(void **buffer, uint32 size) {
  free_obj(buffer, size * sizeof(void *));
}

void *new_void_array(uint32 size) {
  return new_obj(size);
}

void delete_void_array(void *buffer, uint32 size) {
  free_obj(buffer, size);
}

////////////////////////////////////////////////////////////////////////////////

const uint32 MAX_QUEUE_SIZE = 1024;

static void delete_obj(OBJ);

static void release(OBJ *objs, uint32 count, OBJ *queue, uint32 &queue_start, uint32 &queue_size) {
  for (uint32 i=0 ; i < count ; i++) {
    OBJ obj = objs[i];
    if (is_gc_obj(obj)) {
      REF_OBJ *ptr = get_ref_obj_ptr(obj);

      uint32 ref_count = ptr->ref_count;
      assert(ref_count > 0);

      if (ref_count == 1) {
        assert(queue_size <= MAX_QUEUE_SIZE);

        if (queue_size == MAX_QUEUE_SIZE) {
          uint32 idx = queue_start % MAX_QUEUE_SIZE;
          OBJ first_obj = queue[idx];
          queue[idx] = obj;
          queue_start++;
          delete_obj(first_obj);
        }
        else {
          uint32 idx = (queue_start + queue_size) % MAX_QUEUE_SIZE;
          queue[idx] = obj;
          queue_size++;
        }
      }
      else {
        ptr->ref_count = ref_count - 1;
      }
    }
  }
}

static void delete_obj(OBJ obj, OBJ *queue, uint32 &queue_start, uint32 &queue_size) {
  assert(is_gc_obj(obj));

  REF_OBJ *ref_obj = get_ref_obj_ptr(obj);
  OBJ_TYPE obj_type = get_ref_obj_type(obj);

  switch (obj_type) {
    case TYPE_SEQUENCE: {
      SEQ_OBJ *seq = (SEQ_OBJ *) ref_obj;
      release(seq->buffer, seq->size, queue, queue_start, queue_size);
      free_obj(seq, seq_obj_mem_size(seq->capacity));
      break;
    }

    case TYPE_SET: {
      SET_OBJ *set = (SET_OBJ *) ref_obj;
      uint32 size = set->size;
      release(set->buffer, size, queue, queue_start, queue_size);
      free_obj(set, set_obj_mem_size(size));
      break;
    }

    case TYPE_BIN_REL: case TYPE_LOG_MAP: case TYPE_MAP: {
      BIN_REL_OBJ *rel = (BIN_REL_OBJ *) ref_obj;
      uint32 size = rel->size;
      release(rel->buffer, 2*size, queue, queue_start, queue_size);
      free_obj(rel, obj_type == TYPE_MAP ? map_obj_mem_size(size) : bin_rel_obj_mem_size(size));
      break;
    }

    case TYPE_TERN_REL: {
      TERN_REL_OBJ *rel = (TERN_REL_OBJ *) ref_obj;
      uint32 size = rel->size;
      release(rel->buffer, 3*size, queue, queue_start, queue_size);
      free_obj(rel, tern_rel_obj_mem_size(size));
      break;
    }

    case TYPE_TAG_OBJ: {
      TAG_OBJ *tag_obj = (TAG_OBJ *) ref_obj;
      release(&tag_obj->obj, 1, queue, queue_start, queue_size);
      free_obj(tag_obj, tag_obj_mem_size());
      break;
    }

    default:
      internal_fail();
  }
}

static void delete_obj(OBJ obj) {
  assert(is_gc_obj(obj));

  uint32 queue_start = 0;
  uint32 queue_size = 1;
  OBJ queue[MAX_QUEUE_SIZE];
  queue[0] = obj;

  while (queue_size > 0) {
    OBJ next_obj = queue[queue_start % MAX_QUEUE_SIZE];
    queue_size--;
    queue_start++;

    delete_obj(next_obj, queue, queue_start, queue_size);
  }
}

////////////////////////////////////////////////////////////////////////////////

void add_ref(REF_OBJ *ptr) {
#ifndef NOGC
  assert(ptr->ref_count > 0);
  ptr->ref_count++;
#endif
}

void add_ref(OBJ obj) {
#ifndef NOGC
  if (is_gc_obj(obj))
    add_ref(get_ref_obj_ptr(obj));
#endif
}

void release(OBJ obj) {
#ifndef NOGC
  if (is_gc_obj(obj)) {
    REF_OBJ *ptr = get_ref_obj_ptr(obj);
    uint32 ref_count = ptr->ref_count;
    assert(ref_count > 0);
    if (ref_count == 1)
      delete_obj(obj);
    else
      ptr->ref_count = ref_count - 1;
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////

void vec_add_ref(OBJ *objs, uint32 len) {
  for (uint32 i=0 ; i < len ; i++)
    add_ref(objs[i]);
}

void vec_release(OBJ *objs, uint32 len) {
  for (uint32 i=0 ; i < len ; i++)
    release(objs[i]);
}
