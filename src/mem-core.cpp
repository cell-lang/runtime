#include "lib.h"


unsigned int size_code_size(int size_code);
void *alloc_mem_block(int byte_size);
void release_mem_block(void *ptr, int byte_size);

////////////////////////////////////////////////////////////////////////////////

int min_size_code_ref(uint32 byte_size) {
  if (byte_size <= 64)
    return 0;
  if (byte_size <= 128)
    return 1;
  if (byte_size <= 256)
    return 2;
  if (byte_size <= 512)
    return 3;
  if (byte_size <= 1024)
    return 4;
  if (byte_size <= 2048)
    return 5;

  for (int i=0 ; i < 20 ; i++)
    if (byte_size <= 4096 << i)
      return -(1 << i);

  return -(1 << 20);
}

int min_size_code_fast(uint32 byte_size) {
  const uint64 SLOT_MASK =
    (0ULL <<  1 * 4) |  //   64
    (1ULL <<  2 * 4) |  //  128
    (2ULL <<  3 * 4) |  //  196
    (2ULL <<  4 * 4) |  //  256
    (3ULL <<  5 * 4) |  //  320
    (3ULL <<  6 * 4) |  //  384
    (3ULL <<  7 * 4) |  //  448
    (3ULL <<  8 * 4) |  //  512
    (4ULL <<  9 * 4) |  //  576
    (4ULL << 10 * 4) |  //  640
    (4ULL << 11 * 4) |  //  704
    (4ULL << 12 * 4) |  //  768
    (4ULL << 13 * 4) |  //  832
    (4ULL << 14 * 4) |  //  896
    (4ULL << 15 * 4);   //  960

  assert(byte_size > 0);

  if (byte_size <= 960) {
    int blocks_64_count = (byte_size + 63) / 64;
    assert(blocks_64_count < 16);
    return (SLOT_MASK >> (4 * blocks_64_count)) & 7;
  }

  if (byte_size <= 1024)
    return 4;

  if (byte_size <= 2048)
    return 5;

  for (int i=0 ; i < 20 ; i++)
    if (byte_size <= 4096 << i)
      return -(1 << i);

  return -(1 << 20);
}

int min_size_code(uint32 byte_size) {
  int code = min_size_code_fast(byte_size);
  // int ref_code = min_size_code_ref(byte_size);
  assert(code == min_size_code_ref(byte_size));
  assert(byte_size <= size_code_size(code));
  return code;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG

uint32 num_of_live_objs;
uint32 max_num_of_live_objs;
uint32 total_num_of_objs;

uint32 live_mem_usage;
uint32 max_live_mem_usage;
uint32 total_mem_requested;

std::set<void *> live_objs;

void inc_live_obj_count(uint32 byte_size) {
  num_of_live_objs++;
  total_num_of_objs++;
  if (num_of_live_objs > max_num_of_live_objs)
    max_num_of_live_objs = num_of_live_objs;

  live_mem_usage += byte_size;
  total_mem_requested += byte_size;
  if (live_mem_usage > max_live_mem_usage)
    max_live_mem_usage = live_mem_usage;
}

void dec_live_obj_count(uint32 byte_size) {
  num_of_live_objs--;
  live_mem_usage -= byte_size;
}

uint32 get_live_objs_count() {
  return num_of_live_objs;
}

uint32 get_max_live_objs_count() {
  return max_num_of_live_objs;
}

uint32 get_total_objs_count() {
  return total_num_of_objs;
}

uint32 get_live_mem_usage() {
  return live_mem_usage;
}

uint32 get_max_live_mem_usage() {
  return max_live_mem_usage;
}

uint32 get_total_mem_requested() {
  return total_mem_requested;
}

void print_all_live_objs() {
  if (!live_objs.empty()) {
    fprintf(stderr, "Live objects:\n");
    for (std::set<void*>::iterator it = live_objs.begin() ; it != live_objs.end() ; it++) {
      void *ptr = *it;
      printf("  %8llx\n", (unsigned long long)ptr);
    }
    fflush(stdout);
  }
}

bool is_alive(void *obj) {
  return live_objs.find(obj) != live_objs.end();
}

#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void *new_obj(uint32 byte_size) {
  void *mem_block = alloc_mem_block(min_size_code(byte_size));

#ifndef NDEBUG
  if (!is_in_try_state()) {
    inc_live_obj_count(byte_size); //## THE SIZE IS THE WRONG ONE, BUT IT IS THE SAME THAT IS REPORTED BACK TO free_obj
    live_objs.insert(mem_block);
  }
#endif

  return mem_block;
}

void *new_obj(uint32 byte_size_requested, uint32 &byte_size_returned) {
  int size_code = min_size_code(byte_size_requested);
  byte_size_returned = size_code_size(size_code);
  void *mem_block = alloc_mem_block(size_code);

#ifndef NDEBUG
  if (!is_in_try_state()) {
    inc_live_obj_count(byte_size_returned);
    live_objs.insert(mem_block);
  }
#endif

  return mem_block;
}

void free_obj(void *ptr, uint32 byte_size) {
#ifndef NDEBUG
  if (!is_in_try_state()) {
    assert(num_of_live_objs > 0);
    assert(is_alive(ptr));

    dec_live_obj_count(byte_size);
    live_objs.erase(live_objs.find(ptr));
  }
#endif

  release_mem_block(ptr, min_size_code(byte_size));
}

void* resize_obj(void *ptr, uint32 byte_size, uint32 new_byte_size) {
  void *new_ptr = new_obj(new_byte_size);
  uint32 min_byte_size = byte_size < new_byte_size ? byte_size : new_byte_size;
  memcpy(new_ptr, ptr, min_byte_size);
  free_obj(ptr, byte_size);
  return new_ptr;
}
