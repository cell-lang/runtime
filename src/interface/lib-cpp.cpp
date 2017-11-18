#include "lib.h"


bool table_contains(UNARY_TABLE &table, VALUE_STORE &store, OBJ obj) {
  int64 surr = lookup_value(&store, obj);
  release(obj);
  return surr != -1 && unary_table_contains(&table, surr);
}

bool table_contains(BINARY_TABLE &table, VALUE_STORE &store0, VALUE_STORE &store1, OBJ obj0, OBJ obj1) {
  int64 surr0 = lookup_value(&store0, obj0);
  release(obj0);
  if (surr0 == -1) {
    release(obj1);
    return false;
  }
  int64 surr1 = lookup_value(&store1, obj1);
  release(obj1);
  return surr1 != -1 && binary_table_contains(&table, surr0, surr1);
}

bool table_contains(TERNARY_TABLE &table, VALUE_STORE &store0, VALUE_STORE &store1, VALUE_STORE &store2, OBJ obj0, OBJ obj1, OBJ obj2) {
  int64 surr0 = lookup_value(&store0, obj0);
  release(obj0);
  if (surr0 == -1) {
    release(obj1);
    release(obj2);
    return false;
  }
  int64 surr1 = lookup_value(&store1, obj1);
  release(obj1);
  if (surr1 == -1) {
    release(obj2);
    return false;
  }
  int64 surr2 = lookup_value(&store2, obj2);
  release(obj2);
  return surr2 != -1 && ternary_table_contains(&table, surr0, surr1, surr2);
}
