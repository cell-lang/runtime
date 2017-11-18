#include <vector>
#include <string>
#include <tuple>
#include <memory>
#include <sstream>

#include "cell-lang.h"


using std::vector;
using std::string;
using std::tuple;
using std::unique_ptr;
using std::make_tuple;

/////////////////////////////////// value.cpp //////////////////////////////////

unique_ptr<cell::Value> export_as_value(OBJ);

//////////////////////////////// conversion.cpp ////////////////////////////////

string export_as_std_string(OBJ);

////////////////////////////////// lib-cpp.cpp /////////////////////////////////

bool table_contains(UNARY_TABLE &, VALUE_STORE &, OBJ);
bool table_contains(BINARY_TABLE &, VALUE_STORE &, VALUE_STORE &, OBJ, OBJ);
bool table_contains(TERNARY_TABLE &, VALUE_STORE &, VALUE_STORE &, VALUE_STORE &, OBJ, OBJ, OBJ);

////////////////////////////////////////////////////////////////////////////////

template <typename T> vector<typename T::type> export_as_vector(OBJ obj) {
  assert(is_seq(obj) | is_set(obj));

  vector<typename T::type> result;

  if (is_ne_seq(obj)) {
    uint32 len = get_seq_length(obj);
    OBJ *buffer = get_seq_buffer_ptr(obj);
    result.resize(len);
    for (uint32 i=0 ; i < len ; i++)
      result[i] = T::get_value(buffer[i]);
    return result;
  }
  else if (is_ne_set(obj)) {
    SET_OBJ *ptr = get_set_ptr(obj);
    uint32 size = ptr->size;
    OBJ *buffer = ptr->buffer;
    result.resize(size);
    for (uint32 i=0 ; i < size ; i++)
      result[i] = T::get_value(buffer[i]);
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////

struct bool_conv {
  typedef bool type;
  static bool get_value(OBJ obj) {
    return get_bool(obj);
  }
};

struct int_conv {
  typedef long long type;
  static long long get_value(OBJ obj) {
    return get_int(obj);
  }
};

struct float_conv {
  typedef double type;
  static double get_value(OBJ obj) {
    return get_float(obj);
  }
};

struct symb_conv {
  typedef const char *type;
  static const char *get_value(OBJ obj) {
    return symb_to_raw_str(obj);
  }
};

struct string_conv {
  typedef string type;
  static string get_value(OBJ obj) {
    return export_as_std_string(obj);
  }
};

template <typename T> struct vector_conv {
  typedef vector<typename T::type> type;
  static type get_value(OBJ obj) {
    return export_as_vector<T>(obj);
  }
};

template <typename T> struct tagged_conv {
  typedef typename T::type type;
  static type get_value(OBJ obj) {
    return T::get_value(get_inner_obj(obj));
  }
};

// template <typename... Ts> struct tuple_conv {
//   typedef tuple<typename Ts::type...> type;
//   static type get_value(OBJ obj) {
//   }
// };

struct generic_conv {
  typedef unique_ptr<cell::Value> type;
  static type get_value(OBJ obj) {
    return export_as_value(obj);
  }
};

////////////////////////////////////////////////////////////////////////////////

template <typename T0, typename T1> struct tuple_2_conv {
  typedef tuple<typename T0::type, typename T1::type> type;
  static type get_value(OBJ obj) {
    return make_tuple(
      T0::get_value(at(obj, 0)),
      T1::get_value(at(obj, 1))
    );
  }
};

template <typename T0, typename T1, typename T2> struct tuple_3_conv {
  typedef tuple<typename T0::type, typename T1::type, typename T2::type> type;
  static type get_value(OBJ obj) {
    return make_tuple(
      T0::get_value(at(obj, 0)),
      T1::get_value(at(obj, 1)),
      T2::get_value(at(obj, 2))
    );
  }
};

template <typename T0, typename T1, typename T2, typename T3> struct tuple_4_conv {
  typedef tuple<typename T0::type, typename T1::type, typename T2::type, typename T3::type> type;
  static type get_value(OBJ obj) {
    return make_tuple(
      T0::get_value(at(obj, 0)),
      T1::get_value(at(obj, 1)),
      T2::get_value(at(obj, 2)),
      T3::get_value(at(obj, 3))
    );
  }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4> struct tuple_5_conv {
  typedef tuple<typename T0::type, typename T1::type, typename T2::type, typename T3::type, typename T4::type> type;
  static type get_value(OBJ obj) {
    return make_tuple(
      T0::get_value(at(obj, 0)),
      T1::get_value(at(obj, 1)),
      T2::get_value(at(obj, 2)),
      T3::get_value(at(obj, 3)),
      T4::get_value(at(obj, 4))
    );
  }
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5> struct tuple_6_conv {
  typedef tuple<
    typename T0::type, typename T1::type, typename T2::type,
    typename T3::type, typename T4::type, typename T5::type
  > type;
  static type get_value(OBJ obj) {
    return make_tuple(
      T0::get_value(at(obj, 0)),
      T1::get_value(at(obj, 1)),
      T2::get_value(at(obj, 2)),
      T3::get_value(at(obj, 3)),
      T4::get_value(at(obj, 4)),
      T5::get_value(at(obj, 5))
    );
  }
};

////////////////////////////////////////////////////////////////////////////////

template <typename T> vector<typename T::type> get_unary_rel(UNARY_TABLE &table, VALUE_STORE &store) {
  uint32 size = table.count;
  vector<typename T::type> result(size);
  UNARY_TABLE_ITER iter;
  unary_table_get_iter(&table, &iter);
  for (uint32 i=0 ; i < size ; i++) {
    assert(!unary_table_iter_is_out_of_range(&iter));
    OBJ obj = lookup_surrogate(&store, unary_table_iter_get_field(&iter));
    result[i] = T::get_value(obj);
    release(obj);
    unary_table_iter_next(&iter);
  }
  assert(unary_table_iter_is_out_of_range(&iter));
  return result;
}

template <typename T0, typename T1> vector<tuple<typename T0::type, typename T1::type> >
get_binary_rel(BINARY_TABLE &table, VALUE_STORE &store0, VALUE_STORE &store1, bool flipped) {
  uint32 size = table.left_to_right.size();
  vector<tuple<typename T0::type, typename T1::type> > result(size);
  BINARY_TABLE_ITER iter;
  binary_table_get_iter(&table, &iter);
  for (uint32 i=0 ; i < size ; i++) {
    assert(!binary_table_iter_is_out_of_range(&iter));
    OBJ obj0 = lookup_surrogate(&store0, binary_table_iter_get_left_field(&iter));
    OBJ obj1 = lookup_surrogate(&store1, binary_table_iter_get_right_field(&iter));
    result[i] = make_tuple(T0::get_value(flipped ? obj1 : obj0), T1::get_value(flipped ? obj0 : obj1));
    release(obj0);
    release(obj1);
    binary_table_iter_next(&iter);
  }
  assert(binary_table_iter_is_out_of_range(&iter));
  return result;
}

template <typename T0, typename T1, typename T2> vector<tuple<typename T0::type, typename T1::type, typename T2::type> >
get_ternary_rel(TERNARY_TABLE &table, VALUE_STORE &store0, VALUE_STORE &store1, VALUE_STORE &store2,
    int idx0, int idx1, int idx2) {
  uint32 size = table.unshifted.size();
  vector<tuple<typename T0::type, typename T1::type, typename T2::type> > result(size);
  TERNARY_TABLE_ITER iter;
  ternary_table_get_iter(&table, &iter);
  for (uint32 i=0 ; i < size ; i++) {
    assert(!ternary_table_iter_is_out_of_range(&iter));
    OBJ objs[3];
    objs[0] = lookup_surrogate(&store0, ternary_table_iter_get_left_field(&iter));
    objs[1] = lookup_surrogate(&store1, ternary_table_iter_get_middle_field(&iter));
    objs[2] = lookup_surrogate(&store2, ternary_table_iter_get_right_field(&iter));
    result[i] = make_tuple(T0::get_value(objs[idx0]), T1::get_value(objs[idx1]), T2::get_value(objs[idx2]));
    for (int i=0 ; i < 3 ; i++)
      release(objs[i]);
    ternary_table_iter_next(&iter);
  }
  assert(ternary_table_iter_is_out_of_range(&iter));
  return result;
}

////////////////////////////////////////////////////////////////////////////////

template <typename T> bool lookup_by_left_col(BINARY_TABLE &table, VALUE_STORE &store0, VALUE_STORE &store1, OBJ key, typename T::type &value) {
  int64 surr = lookup_value(&store0, key);
  release(key);
  if (surr == -1)
    return false;
  BINARY_TABLE_ITER iter;
  binary_table_get_iter_by_col_0(&table, &iter, surr);
  if (binary_table_iter_is_out_of_range(&iter))
    return false;
  OBJ obj = lookup_surrogate(&store1, binary_table_iter_get_right_field(&iter));
  value = T::get_value(obj);
  release(obj);
#ifndef NDEBUG
  binary_table_iter_next(&iter);
  assert(binary_table_iter_is_out_of_range(&iter));
#endif
  return true;
}
