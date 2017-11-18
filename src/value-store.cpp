#include "lib.h"


struct NODE {
  uint32 next;
  uint32 hash_code;
};


NODE node(uint32 next_node, uint32 hash_code) {
  NODE node;
  node.next = next_node;
  node.hash_code = hash_code;
  return node;
}

NODE empty_node() {
  return node(0xFFFFFFFFU, 0xFFFFFFFFU);
}

////////////////////////////////////////////////////////////////////////////////

const int BYTES_PER_ENTRY         = sizeof(OBJ) + sizeof(NODE) + sizeof(uint32) + sizeof(uint32);
const int UPDATE_BYTES_PER_ENTRY  = sizeof(OBJ) + sizeof(NODE) + sizeof(uint32) + sizeof(uint32);


OBJ *slot_array(void *ptr) {
  return (OBJ *) ptr;
}

NODE *node_array(void *ptr, uint32 capacity) {
  return (NODE *)(slot_array(ptr) + capacity);
}

uint32 *hashtable_ptr(void *ptr, uint32 capacity) {
  return (uint32 *)(node_array(ptr, capacity) + capacity);
}

uint32 *ref_count_array(void *ptr, uint32 capacity) {
  return (uint32 *)(hashtable_ptr(ptr, capacity) + capacity);
}

uint32 *surr_array(void *ptr, uint32 capacity) {
  return (uint32 *)(hashtable_ptr(ptr, capacity) + capacity);
}

////////////////////////////////////////////////////////////////////////////////

const uint32 EMPTY_SLOT_MARKER = 0xFFFFFFFFU;


void reset_slot(OBJ *slot, uint32 first_free) {
  slot->core_data.int_ = first_free;
  slot->extra_data = 0; // Should not be necessary
  assert(get_physical_type(*slot) == TYPE_BLANK_OBJ);
}

static void hashtable_clear(void *ptr, uint32 capacity) {
  OBJ *slots = slot_array(ptr);
  for (uint32 i=0 ; i < capacity ; i++)
    reset_slot(slots+i, i+1);
  uint32 *hash_table = hashtable_ptr(ptr, capacity);
  memset(hash_table, 0xFF, capacity * sizeof(uint32));
  // Initializing the array of node is not stricly necessary
  NODE *nodes = node_array(ptr, capacity);
  for (uint32 i=0 ; i < capacity ; i++)
    nodes[i] = empty_node();
}

static void hashtable_insert(void *ptr, uint32 capacity, uint32 hash_code, uint32 value) {
  uint32 *hashtable = hashtable_ptr(ptr, capacity);
  NODE *nodes = node_array(ptr, capacity);
  uint32 index = hash_code % capacity;
  uint32 entry = hashtable[index];
  hashtable[index] = value;
  nodes[value] = node(entry, hash_code);
}

static void hashtable_delete(void *ptr, uint32 capacity, uint32 value) {
  uint32 *hashtable = hashtable_ptr(ptr, capacity);
  NODE *nodes = node_array(ptr, capacity);
  NODE del_node = nodes[value];
  nodes[value] = empty_node(); // Not strictly necessary
  uint32 index = del_node.hash_code % capacity;
  uint32 entry = hashtable[index];
  assert(entry >= 0 & entry < capacity);
  // We first check to see if the deleted node is the first one in the list
  if (entry == value) {
    hashtable[index] = del_node.next; // Note that <next> can be EMPTY_SLOT_MARKER
    return;
  }
  // Here <entry> is the index of the current cell
  // We already know that the current cell is not the one we're looking for.
  NODE node = nodes[entry];
  while (node.next != value) {
    entry = node.next;
    assert(entry >= 0 & entry < capacity);
    node = nodes[entry];
  }
  // We've finally reached the cell before the one we are deleting
  // Again <entry> is the index of the current cell, and <node> is its content
  node.next = del_node.next;
  nodes[entry] = node;
}

static int64 hashtable_lookup(void *ptr, uint32 capacity, OBJ value, uint32 hash_code) {
  OBJ *slots = slot_array(ptr);
  NODE *nodes = node_array(ptr, capacity);
  uint32 *hashtable = hashtable_ptr(ptr, capacity);
  uint32 index = hash_code % capacity;
  uint32 entry = hashtable[index];
  while (entry != EMPTY_SLOT_MARKER) {
    NODE node = nodes[entry];
    if (node.hash_code == hash_code && comp_objs(value, slots[entry]) == 0)
      return entry;
    entry = node.next;
  }
  return -1;
}

static void hashtable_copy(void *src_ptr, uint32 src_cpty, void *dest_ptr, uint32 dest_cpty) {
  assert(dest_cpty > src_cpty);

  OBJ *src_slots = slot_array(src_ptr);
  OBJ *dest_slots = slot_array(dest_ptr);
  memcpy(dest_slots, src_slots, src_cpty * sizeof(OBJ));
  for (uint32 i=src_cpty ; i < dest_cpty ; i++)
    reset_slot(dest_slots+i, i+1);

  uint32 *dest_hashtable = hashtable_ptr(dest_ptr, dest_cpty);
  memset(dest_hashtable, 0xFF, dest_cpty * sizeof(uint32));

  // Initializing the new array of node is not stricly necessary
  NODE *dest_nodes = node_array(dest_ptr, dest_cpty);
  for (uint32 i=0 ; i < dest_cpty ; i++)
    dest_nodes[i] = empty_node();

  NODE *src_nodes = node_array(src_ptr, src_cpty);
  for (uint32 i=0 ; i < src_cpty ; i++)
    if (!is_blank_obj(src_slots[i]))
      hashtable_insert(dest_ptr, dest_cpty, src_nodes[i].hash_code, i);
}

// int64 ref_hashtable_lookup(void *ptr, uint32 capacity, OBJ value, uint32 hash_code) {
//   OBJ *slots = slot_array(ptr);
//   for (uint32 i=0 ; i < capacity ; i++) {
//     OBJ slot = slots[i];
//     if (!is_blank_obj(slot) && comp_objs(value, slot) == 0)
//       return i;
//   }
//   return -1;
// }
//
// int64 hashtable_lookup(void *ptr, uint32 capacity, OBJ value, uint32 hash_code) {
//   int64 ref_res = ref_hashtable_lookup(ptr, capacity, value, hash_code);
//   int64 res = hashtable_lookup_(ptr, capacity, value, hash_code);
//   assert(res == ref_res);
//   if (res != ref_res)
//     fail();
//   return res;
// }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

const uint32 INIT_SIZE = 4;

uint32 calc_capacity(uint32 min_capacity) {
  uint32 capacity = INIT_SIZE;
  while (capacity < min_capacity)
    capacity *= 2;
  return capacity;
}

////////////////////////////////////////////////////////////////////////////////

void value_store_init(VALUE_STORE *store) {
  void *ptr = new_obj(INIT_SIZE * BYTES_PER_ENTRY);
  store->ptr = ptr;
  store->capacity = INIT_SIZE;
  store->usage = 0;
  store->first_free = 0;
  hashtable_clear(ptr, INIT_SIZE);
  memset(ref_count_array(ptr, INIT_SIZE), 0, INIT_SIZE * sizeof(uint32));
}

void value_store_cleanup(VALUE_STORE *store) {
  uint32 capacity = store->capacity;
  OBJ *slots = slot_array(store->ptr);
  for (uint32 i=0 ; i < capacity ; i++)
    release(slots[i]);
  free_obj(slots, capacity * BYTES_PER_ENTRY);
}

////////////////////////////////////////////////////////////////////////////////

void value_store_updates_init(VALUE_STORE *store, VALUE_STORE_UPDATES *updates) {
  updates->capacity = 0;
  // Not strictly necessary
  updates->ptr = NULL;
  updates->count = 0;
  updates->first_free = 0;
}

void value_store_updates_cleanup(VALUE_STORE_UPDATES *updates) {
  // No need to delete anything, it's all allocated in temporary memory
}

////////////////////////////////////////////////////////////////////////////////

uint32 value_store_insert(VALUE_STORE *store, VALUE_STORE_UPDATES *updates, OBJ value) {
  uint32 hash_code = compute_hash_code(value);

  void *ptr = updates->ptr;
  uint32 capacity = updates->capacity;
  uint32 count = updates->count;
  assert(count <= capacity);

  if (count == capacity) {
    uint32 new_capacity = capacity != 0 ? 2 * capacity : 32;
    void *new_ptr = new_obj(new_capacity * UPDATE_BYTES_PER_ENTRY);
    if (capacity > 0) {
      hashtable_copy(ptr, capacity, new_ptr, new_capacity);
      uint32 *surrs = surr_array(ptr, capacity);
      uint32 *new_surrs = surr_array(new_ptr, new_capacity);
      memcpy(new_surrs, surrs, capacity * sizeof(uint32));
      memset(new_surrs + capacity, 0, (new_capacity - capacity) * sizeof(uint32));
      free_obj(ptr, capacity * UPDATE_BYTES_PER_ENTRY);
    }
    else
      hashtable_clear(new_ptr, new_capacity);
    updates->capacity = capacity = new_capacity;
    updates->ptr = ptr = new_ptr;
  }

  OBJ *values = slot_array(ptr);
  values[count] = value;
  hashtable_insert(ptr, capacity, hash_code, count);
  uint32 first_free = count == 0 ? store->first_free : updates->first_free;
  uint32 *surrs = surr_array(ptr, capacity);
  surrs[count] = first_free;
  updates->count = count + 1;
  if (first_free < store->capacity)
    updates->first_free = slot_array(store->ptr)[first_free].core_data.int_;
  else
    updates->first_free = first_free + 1;
  return first_free;
}

////////////////////////////////////////////////////////////////////////////////

void value_store_copy(VALUE_STORE *store, VALUE_STORE_UPDATES *updates) {

}

void value_store_apply(VALUE_STORE *store, VALUE_STORE_UPDATES *updates) {
  if (updates->capacity == 0)
    return;

  uint32 store_capacity = store->capacity;
  void *ptr = store->ptr;
  uint32 usage = store->usage;

  uint32 count = updates->count;
  uint32 new_usage = usage + count;

  if (store_capacity < new_usage) {
    uint32 new_capacity = calc_capacity(new_usage);
    void *new_ptr = new_obj(new_capacity * BYTES_PER_ENTRY);
    hashtable_copy(ptr, store_capacity, new_ptr, new_capacity);
    uint32 *ref_counts = ref_count_array(ptr, store_capacity);
    uint32 *new_ref_counts = ref_count_array(new_ptr, new_capacity);
    memcpy(new_ref_counts, ref_counts, store_capacity * sizeof(uint32));
    memset(new_ref_counts+store_capacity, 0, (new_capacity-store_capacity) * sizeof(uint32));
    free_obj(ptr, store_capacity * BYTES_PER_ENTRY);
    store->ptr = ptr = new_ptr;
    store->capacity = store_capacity = new_capacity;
  }

  OBJ *slots = slot_array(ptr);

  uint32 update_cpty = updates->capacity;
  void *update_ptr = updates->ptr;
  OBJ *values = slot_array(update_ptr);
  NODE *nodes = node_array(update_ptr, update_cpty);
  uint32 *surrs = surr_array(update_ptr, update_cpty);
  for (uint32 i=0 ; i < count ; i++) {
    uint32 surr = surrs[i];
    slots[surr] = copy_obj(values[i]);
    hashtable_insert(ptr, store_capacity, nodes[i].hash_code, surr);
  }
  store->usage = new_usage;
  store->first_free = updates->first_free;
}

void value_store_add_ref(VALUE_STORE *store, uint32 surr) {
  assert(surr < store->capacity);
  uint32 *ref_counts = ref_count_array(store->ptr, store->capacity);
  ref_counts[surr]++;
}

void value_store_release(VALUE_STORE *store, uint32 surr) {
  void *ptr = store->ptr;
  uint32 capacity = store->capacity;
  assert(surr < store->capacity);
  uint32 *ref_counts = ref_count_array(ptr, capacity);
  uint32 count = ref_counts[surr];
  assert(count != 0);
  if (count == 1) {
    OBJ *slot = slot_array(ptr) + surr;
    release(*slot);
    reset_slot(slot, store->first_free);
    store->first_free = surr;
    store->usage--;
    hashtable_delete(ptr, capacity, surr);
  }
  else
    ref_counts[surr] = count - 1;
}

////////////////////////////////////////////////////////////////////////////////

OBJ lookup_surrogate(VALUE_STORE *store, int64 surr) {
  OBJ value = slot_array(store->ptr)[surr];
  add_ref(value);
  return value;
}

int64 lookup_value(VALUE_STORE *store, OBJ value) {
  return hashtable_lookup(store->ptr, store->capacity, value, compute_hash_code(value));
}

////////////////////////////////////////////////////////////////////////////////

int64 lookup_value_ex(VALUE_STORE *store, VALUE_STORE_UPDATES *updates, OBJ value) {
  uint32 hash_code = compute_hash_code(value);
  int64 surr = hashtable_lookup(store->ptr, store->capacity, value, hash_code);
  if (surr != -1)
    return surr;
  uint32 capacity = updates->capacity;
  if (capacity > 0) {
    void *ptr = updates->ptr;
    int64 index = hashtable_lookup(ptr, capacity, value, hash_code);
    if (index >= 0)
      return surr_array(ptr, capacity)[index];
  }
  return -1;
}

////////////////////////////////////////////////////////////////////////////////

OBJ *value_store_slot_array(VALUE_STORE *store) {
  return slot_array(store->ptr);
}
