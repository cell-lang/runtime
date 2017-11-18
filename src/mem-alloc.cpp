#include "lib.h"


void *alloc_pages(unsigned int page_count) {
  assert(page_count > 0);
  void *ptr = malloc(4096 * page_count);
  // printf("+ %8llx - %4d\n", (unsigned long long) ptr, page_count);
  return ptr;
}

void release_pages(void *ptr, unsigned int page_count) {
  assert(ptr != NULL & page_count > 0);
  // printf("- %8llx - %4d\n", (unsigned long long) ptr, page_count);
  free(ptr);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int size_code_size(int size_code) {
  assert(size_code <= 5);
  switch (size_code) {
    case 0:   return 64;
    case 1:   return 128;
    case 2:   return 256;
    case 3:   return 512;
    case 4:   return 1024;
    case 5:   return 2048;
    default:  return 4096 * -size_code;
  }
}

////////////////////////////////////////////////////////////////////////////////

enum MEM_ALLOC_STATE {NORMAL, TRY, COPYING};

static MEM_ALLOC_STATE curr_mem_alloc_state = NORMAL;


const int SLOT_COUNT = 6;

struct STD_MEM_ALLOC {
  void *mem_blocks_pool[SLOT_COUNT];
};

struct TRY_STATE_MEM_ALLOC {
  void *mem_blocks_pool[SLOT_COUNT];
  std::vector<void *> pooled_blocks;
  std::map<void *, unsigned int> large_blocks;
};

static STD_MEM_ALLOC       std_mem_alloc;
static TRY_STATE_MEM_ALLOC try_mem_alloc;

static void **curr_mem_blocks_pool = std_mem_alloc.mem_blocks_pool;

////////////////////////////////////////////////////////////////////////////////

bool is_in_normal_state() {
  return curr_mem_alloc_state == NORMAL;
}

bool is_in_try_state() {
  return curr_mem_alloc_state == TRY;
}

bool is_in_copying_state() {
  return curr_mem_alloc_state == COPYING;
}

////////////////////////////////////////////////////////////////////////////////

void enter_try_state() {
  assert(curr_mem_alloc_state == NORMAL);

  curr_mem_alloc_state = TRY;
  curr_mem_blocks_pool = try_mem_alloc.mem_blocks_pool;
}

void enter_copy_state() {
  assert(curr_mem_alloc_state == TRY);

  curr_mem_alloc_state = COPYING;
  curr_mem_blocks_pool = std_mem_alloc.mem_blocks_pool;
}

void restore_try_state() {
  assert(curr_mem_alloc_state == COPYING);

  curr_mem_alloc_state = TRY;
  curr_mem_blocks_pool = try_mem_alloc.mem_blocks_pool;
}

void release_all_try_state_memory() {
  std::vector<void *>::iterator pit = try_mem_alloc.pooled_blocks.begin();
  std::vector<void *>::iterator pend = try_mem_alloc.pooled_blocks.end();
  for ( ; pit != pend ; pit++)
    release_pages(*pit, 16);
  try_mem_alloc.pooled_blocks.clear();

  std::map<void *, unsigned int>::iterator lit = try_mem_alloc.large_blocks.begin();
  std::map<void *, unsigned int>::iterator lend = try_mem_alloc.large_blocks.end();
  for ( ; lit != lend ; lit++)
    release_pages(lit->first, lit->second);
  try_mem_alloc.large_blocks.clear();

  for (int i=0 ; i < SLOT_COUNT ; i++)
    try_mem_alloc.mem_blocks_pool[i] = NULL;
}

void return_to_normal_state() {
  assert(curr_mem_alloc_state == COPYING);

  curr_mem_alloc_state = NORMAL;
  release_all_try_state_memory();
}

void abort_try_state() {
  assert(curr_mem_alloc_state == TRY);

  curr_mem_alloc_state = NORMAL;
  curr_mem_blocks_pool = std_mem_alloc.mem_blocks_pool;
  release_all_try_state_memory();
}

////////////////////////////////////////////////////////////////////////////////

void *alloc_mem_block(int size_code) {
  assert(size_code <= 5);

  if (size_code >= 0) {
    void **pool_head = curr_mem_blocks_pool + size_code;

    void *head = *pool_head;
    if (head == NULL) {
      // Allocate new memory block
      void *ptr = alloc_pages(16);
      if (curr_mem_alloc_state == TRY)
        try_mem_alloc.pooled_blocks.push_back(ptr);
#ifndef NDEBUG
      memset(ptr, 0xFF, 16 * 4096);
#endif
      int log_size = size_code + 6;
      int block_size = size_code_size(size_code);
      assert(
        (size_code == 0 & block_size ==   64) |
        (size_code == 1 & block_size ==  128) |
        (size_code == 2 & block_size ==  256) |
        (size_code == 3 & block_size ==  512) |
        (size_code == 4 & block_size == 1024) |
        (size_code == 5 & block_size == 2048)
      );

      unsigned int last_block_idx = ((16 * 4096) >> log_size) - 1;
      assert(last_block_idx + 1 == (16 * 4096) / block_size);
      for (int i=1 ; i < last_block_idx ; i++) {
        void *block_ptr = ((char *) ptr) + (i << log_size);
        assert(block_ptr == ((char *) ptr) + i * block_size);
        * (void **) block_ptr = ((char *) block_ptr) + block_size;
      }
      void *last_block_ptr = ((char *) ptr) + (last_block_idx << log_size);
      assert(last_block_ptr == ((char *) ptr) + (last_block_idx * block_size));
      * (void **) last_block_ptr = NULL;
      *pool_head = ((char *) ptr) + block_size;
      return ptr;
    }
    else {
      void *next = * (void **) head;
      *pool_head = next;
      return head;
    }
  }
  else {
    void *ptr = alloc_pages(-size_code);
    if (curr_mem_alloc_state == TRY)
      try_mem_alloc.large_blocks[ptr] = -size_code;
#ifndef NDEBUG
    memset(ptr, 0xFF, -size_code * 4096);
#endif
    return ptr;
  }
}

void release_mem_block(void *ptr, int size_code) {
  assert(size_code <= 5);

  if (size_code >= 0) {
#ifndef NDEBUG
    unsigned int block_size = size_code_size(size_code);
    memset(ptr, 0xFF, block_size);
#endif
    void **pool_head = curr_mem_blocks_pool + size_code;
    void *tail = *pool_head;
    * (void **) ptr = tail;
    *pool_head = ptr;
  }
  else {
    release_pages(ptr, -size_code);
    if (curr_mem_alloc_state == TRY)
      try_mem_alloc.large_blocks.erase(ptr);
  }
}
