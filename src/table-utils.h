inline uint64 pack(uint64 left, uint64 right) {
  return (left << 32) | right;
}

inline uint64 swap(uint64 pair) {
  return (pair >> 32) | (pair << 32);
}

inline uint32 left(uint64 pair) {
  return pair >> 32;
}

inline uint32 right(uint64 pair) {
  return pair;
}

template <typename T> void sort_unique(std::vector<T> &xs) {
  std::sort(xs.begin(), xs.end());
  xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
}

struct col_0 {
  typedef uint32 key_type;
  static uint32 key(uint64 tuple) {
    return left(tuple);
  }
  static uint32 key_shifted(uint64 unshifted_tuple) {
    return left(unshifted_tuple);
  }
  static uint64 lower_bound(uint32 key) {
    return pack(key, 0);
  }
  static const bool SORTED = true;
};


struct col_1 {
  typedef uint32 key_type;
  static uint32 key(uint64 tuple) {
    return right(tuple);
  }
  static uint32 key_shifted(uint64 flipped_tuple) {
    return left(flipped_tuple);
  }
  static uint64 lower_bound(uint32 key) {
    return pack(key, 0);
  }
  static const bool SORTED = false;
};

////////////////////////////////////////////////////////////////////////////////

inline void build(tuple3 &tuple, uint32 val0, uint32 val1, uint32 val2) {
  tuple.fields01 = pack(val0, val1);
  tuple.field2 = val2;
}

inline void shift(tuple3 &tuple) {
  uint32 new_field2 = left(tuple.fields01);
  tuple.fields01 = pack(tuple.fields01, tuple.field2);
  tuple.field2 = new_field2;
}

inline tuple3 lower_bound(uint64 key) {
  tuple3 lb;
  lb.fields01 = key;
  lb.field2 = 0;
  return lb;
}

struct cols_01 {
  typedef uint64 key_type;
  static uint64 key(const tuple3 &tuple) {
    return tuple.fields01;
  }
  static uint64 key_shifted(const tuple3 &unshifted_tuple) {
    return unshifted_tuple.fields01;
  }
  static tuple3 lower_bound(uint64 key) {
    return ::lower_bound(key);
  }
  static const bool SORTED = true;
};

struct cols_12 {
  typedef uint64 key_type;
  static uint64 key(const tuple3 &tuple) {
    return pack(right(tuple.fields01), tuple.field2);
  }
  static uint64 key_shifted(const tuple3 &shifted_once_tuple) {
    return shifted_once_tuple.fields01;
  }
  static tuple3 lower_bound(uint64 key) {
    return ::lower_bound(key);
  }
  static const bool SORTED = false;
};

struct cols_20 {
  typedef uint64 key_type;
  static uint64 key(const tuple3 &tuple) {
    return pack(tuple.field2, left(tuple.fields01));
  }
  static uint64 key_shifted(const tuple3 &shifted_twice_tuple) {
    return shifted_twice_tuple.fields01;
  }
  static tuple3 lower_bound(uint64 key) {
    return ::lower_bound(key);
  }
  static const bool SORTED = false;
};

struct col_2 {
  typedef uint32 key_type;
  static uint32 key(const tuple3 &tuple) {
    return tuple.field2;
  }
  static uint32 key_shifted(const tuple3 &shifted_twice_tuple) {
    return left(shifted_twice_tuple.fields01);
  }
  static tuple3 lower_bound(uint32 key) {
    return ::lower_bound(pack(key, 0));
  }
  static const bool SORTED = false;
};

////////////////////////////////////////////////////////////////////////////////

template <typename K, typename T> void take_keys(std::vector<typename K::key_type> &keys, const std::vector<T> &tuples) {
  uint32 count = tuples.size();
  keys.resize(count);
  for (uint32 i=0 ; i < count ; i++)
    keys[i] = K::key(tuples[i]);
}

template <typename T> bool sorted_vector_has_duplicates(std::vector<T> &xs) {
  uint32 count = xs.size();
  if (count > 0) {
    uint64 last_x = xs[0];
    for (int i=1 ; i < count ; i++) {
      uint64 x = xs[i];
      if (x == last_x)
        return true;
      assert(x > last_x);
      last_x = x;
    }
  }
  return false;
}

template <typename K, typename T>
bool update_has_conflicts(std::vector<typename K::key_type> &inserted_keys, std::vector<typename K::key_type> &deleted_keys, std::set<T> &target) {
  int count = inserted_keys.size();
  for (int i=0 ; i < count ; i++) {
    typename K::key_type key = inserted_keys[i];
    if (!binary_search(deleted_keys.begin(), deleted_keys.end(), key)) {
      T lb = K::lower_bound(key);
      typename std::set<T>::iterator it = target.lower_bound(lb);
      if (it != target.end() && K::key_shifted(*it) == key)
        return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

template <typename K, typename T>
bool table_updates_check_key(const std::vector<T> &inserts, const std::vector<T> &deletes, std::set<T> &target) {
  // Gathering and sorting all keys from tuples to delete
  std::vector<typename K::key_type> deleted_keys;
  take_keys<K>(deleted_keys, deletes);
  std::sort(deleted_keys.begin(), deleted_keys.end());

  // Gathering all keys from tuples to insert and sorting them if they are not naturally sorted
  std::vector<typename K::key_type> inserted_keys;
  take_keys<K>(inserted_keys, inserts);
  if (not K::SORTED)
    std::sort(inserted_keys.begin(), inserted_keys.end());

  // Checking that there are no duplicates. Since the duplicates among the tuples
  // to insert have already been eliminated, the presence of a duplicate among
  // the keys implies a unicity conflict
  if (sorted_vector_has_duplicates(inserted_keys))
    return false;

  // Checking that for each key to insert, either there's a corresponding
  // entry among the values to delete, or there's no entry in the current table
  return not update_has_conflicts<K>(inserted_keys, deleted_keys, target);
}
