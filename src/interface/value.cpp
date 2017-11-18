#include "lib.h"
#include "lib-cpp.h"

using std::string;
using std::to_string;
using std::ostream;
using std::ostringstream;

using cell::Value;


class ValueBase : public Value {
public:
  bool is_symb();
  bool is_int();
  bool is_float();
  bool is_seq();
  bool is_set();
  bool is_bin_rel();
  bool is_tern_rel();
  bool is_tagged();

  const char *as_symb();
  long long as_int();
  double as_float();

  unsigned int size();
  Value *item(unsigned int);
  void entry(unsigned int, Value *&, Value *&);
  void entry(unsigned int, Value *&, Value *&, Value *&);

  const char *tag();
  Value *untagged();

  bool is_string();
  bool is_record();

  string as_str();
  Value *lookup(const char *);

  string printed();

protected:
  ValueBase();
};


class SymbValue : public ValueBase {
public:
  SymbValue(const char *);

  bool is_symb();
  const char *as_symb();
  void print(ostream &);

private:
  const char *ptr;
};


class IntValue : public ValueBase {
public:
  IntValue(long long);

  bool is_int();
  long long as_int();
  void print(ostream &);

private:
  long long value;
};


class FloatValue : public ValueBase {
public:
  FloatValue(double);

  bool is_float();
  double as_float();
  void print(ostream &);

private:
  double value;
};


class SeqSetValue : public ValueBase {
public:
  SeqSetValue(Value **, unsigned int, bool);
  ~SeqSetValue();

  bool is_seq();
  bool is_set();
  bool is_bin_rel();
  bool is_tern_rel();
  unsigned int size();
  Value *item(unsigned int);
  void print(ostream &);

private:
  Value **items;
  unsigned int count;
  bool ordered;
};


class BinRelValue : public ValueBase {
public:
  BinRelValue(Value *(*)[2], unsigned int, bool);
  ~BinRelValue();

  bool is_bin_rel();
  unsigned int size();
  void entry(unsigned int, Value *&, Value *&);
  bool is_record();
  Value *lookup(const char *);
  void print(ostream &);

private:
  Value *(*entries)[2];
  unsigned int count;
  bool is_map;
  bool is_rec;
};


class TernRelValue : public ValueBase {
public:
  TernRelValue(Value *(*)[3], unsigned int);
  ~TernRelValue();

  bool is_tern_rel();
  unsigned int size();
  void entry(unsigned int, Value *&, Value *&, Value *&);
  void print(ostream &);

private:
  Value *(*entries)[3];
  unsigned int count;
};


class TaggedValue : public ValueBase {
public:
  TaggedValue(const char *, Value *);
  ~TaggedValue();

  bool is_tagged();
  const char *tag();
  Value *untagged();
  bool is_string();
  string as_str();
  Value *lookup(const char *);
  void print(ostream &);

private:
  const char *tag_ptr;
  Value *value;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ValueBase::ValueBase() {

}

bool ValueBase::is_symb() {
  return false;
}

bool ValueBase::is_int() {
  return false;
}

bool ValueBase::is_float() {
  return false;
}

bool ValueBase::is_seq() {
  return false;
}

bool ValueBase::is_set() {
  return false;
}

bool ValueBase::is_bin_rel() {
  return false;
}

bool ValueBase::is_tern_rel() {
  return false;
}

bool ValueBase::is_tagged() {
  return false;
}

const char *ValueBase::as_symb() {
  throw 0LL;
}

long long ValueBase::as_int() {
  throw 0LL;
}

double ValueBase::as_float() {
  throw 0LL;
}

unsigned int ValueBase::size() {
  throw 0LL;
}

Value *ValueBase::item(unsigned int) {
  throw 0LL;
}

void ValueBase::entry(unsigned int, Value *&, Value *&) {
  throw 0LL;
}

void ValueBase::entry(unsigned int, Value *&, Value *&, Value *&) {
  throw 0LL;
}

const char *ValueBase::tag() {
  throw 0LL;
}

Value *ValueBase::untagged() {
  throw 0LL;
}

bool ValueBase::is_string() {
  return false;
}

bool ValueBase::is_record() {
  return false;
}

string ValueBase::as_str() {
  throw 0LL;
}

Value *ValueBase::lookup(const char *) {
  throw 0LL;
}

string ValueBase::printed() {
  ostringstream os;
  print(os);
  return os.str();
}

////////////////////////////////////////////////////////////////////////////////

SymbValue::SymbValue(const char *ptr) : ptr(ptr) {

}

bool SymbValue::is_symb() {
  return true;
}

const char *SymbValue::as_symb() {
  return ptr;
}

void SymbValue::print(ostream &os) {
  os << ptr;
}

////////////////////////////////////////////////////////////////////////////////

IntValue::IntValue(long long value) : value(value) {

}

bool IntValue::is_int() {
  return true;
}

long long IntValue::as_int() {
  return value;
}

void IntValue::print(ostream &os) {
  os << value;
}

////////////////////////////////////////////////////////////////////////////////

FloatValue::FloatValue(double value) : value(value) {

}

bool FloatValue::is_float() {
  return true;
}

double FloatValue::as_float() {
  return value;
}

void FloatValue::print(ostream &os) {
  os << value;
}

////////////////////////////////////////////////////////////////////////////////

SeqSetValue::SeqSetValue(Value **items, unsigned int count, bool ordered)
: items(items), count(count), ordered(ordered) {

}

SeqSetValue::~SeqSetValue() {
  delete [] items;
}


bool SeqSetValue::is_seq() {
  return ordered;
}

bool SeqSetValue::is_set() {
  return !ordered;
}

bool SeqSetValue::is_bin_rel() {
  return !ordered & count == 0;
}

bool SeqSetValue::is_tern_rel() {
  return !ordered & count == 0;
}

unsigned int SeqSetValue::size() {
  return count;
}

Value *SeqSetValue::item(unsigned int idx) {
  if (idx < count)
    return items[idx];
  else
    throw 0LL;
}

void SeqSetValue::print(ostream &os) {
  os << (ordered ? "(" : "[");
  for (uint32 i=0 ; i < count ; i++) {
    if (i > 0)
      os << ", ";
    items[i]->print(os);
  }
  os << (ordered ? ")" : "]");
}

////////////////////////////////////////////////////////////////////////////////

BinRelValue::BinRelValue(Value *(*entries)[2], unsigned int count, bool is_map)
: entries(entries), count(count), is_map(is_map) {
  is_rec = false;
  for (uint32 i=0 ; i < count ; i++)
    if (!entries[i][0]->is_symb())
      return;
  is_rec = is_map & count > 0;
}

BinRelValue::~BinRelValue() {
  delete [] entries;
}

bool BinRelValue::is_bin_rel() {
  return true;
}

unsigned int BinRelValue::size() {
  return count;
}

void BinRelValue::entry(unsigned int idx, Value *&arg0, Value *&arg1) {
  if (idx < count) {
    arg0 = entries[idx][0];
    arg1 = entries[idx][1];
  }
  else
    throw 0LL;
}

bool BinRelValue::is_record() {
  return is_rec;
}

Value *BinRelValue::lookup(const char *field) {
  for (uint32 i=0 ; i < count ; i++) {
    Value *key = entries[i][0];
    if (key->is_symb() && strcmp(field, key->as_symb()) == 0)
      return entries[i][1];
  }
  throw 0LL;
}

void BinRelValue::print(ostream &os) {
  if (count > 0) {
    os << (is_rec ? "(" : "[");
    for (uint32 i=0 ; i < count ; i++) {
      os << (i == 0 ? "" : is_map ? ", " : " ");
      entries[i][0]->print(os);
      os << (is_rec ? ": " : is_map ? " -> " : ", ");
      entries[i][1]->print(os);
      os << (is_map ? "" : ";");
    }
    os << (is_rec ? ")" : "]");
  }
  else
    os << "[:]";
}

////////////////////////////////////////////////////////////////////////////////

TernRelValue::TernRelValue(Value *(*entries)[3], unsigned int count) : entries(entries), count(count) {

}

TernRelValue::~TernRelValue() {
  delete [] entries;
}

bool TernRelValue::is_tern_rel() {
  return true;
}

unsigned int TernRelValue::size() {
  return count;
}

void TernRelValue::entry(unsigned int idx, Value *&arg0, Value *&arg1, Value *&arg2) {
  if (idx < count) {
    Value **entry = entries[count];
    arg0 = entry[0];
    arg1 = entry[1];
    arg2 = entry[2];
  }
  else
    throw 0LL;
}

void TernRelValue::print(ostream &os) {
  os << "[";
  for (uint32 i=0 ; i < count ; i++) {
    if (i > 0);
      os << " ";
    entries[i][0]->print(os);
    os << ", ";
    entries[i][1]->print(os);
    os << ", ";
    entries[i][2]->print(os);
    os << ";";
  }
  os << "]";
}

////////////////////////////////////////////////////////////////////////////////

TaggedValue::TaggedValue(const char *tag_ptr, Value *value) : tag_ptr(tag_ptr), value(value) {

}

TaggedValue::~TaggedValue() {
  delete value;
}

bool TaggedValue::is_tagged() {
  return true;
}

const char *TaggedValue::tag() {
  return tag_ptr;
}

Value *TaggedValue::untagged() {
  return value;
}

bool TaggedValue::is_string() {
  if (strcmp(tag_ptr, "string") != 0 || !value->is_seq())
    return false;
  uint32 len = value->size();
  for (uint32 i=0 ; i < len ; i++) {
    Value *item = value->item(i);
    if (!item->is_int())
      return false;
    long long int_val = item->as_int();
    if (int_val < 0 | int_val > 1114111)
      return false;
  }
  return true;
}

string TaggedValue::as_str() {
  if (strcmp(tag_ptr, "string") != 0)
    throw 0LL;
  string result;
  uint32 len = value->size();
  for (uint32 i=0 ; i < len ; i++)
    result.push_back(value->item(i)->as_int());
  return result;
}

Value *TaggedValue::lookup(const char *field) {
  return value->lookup(field);
}

void TaggedValue::print(ostream &os) {
  bool skip_pars = value->is_record() | (value->is_seq() && value->size() > 0);
  os << tag_ptr;
  if (!skip_pars)
    os << "(";
  value->print(os);
  if (!skip_pars)
    os << ")";
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Value *export_as_value_ptr(OBJ obj) {
  if (is_tag_obj(obj))
    return new TaggedValue(symb_to_raw_str(get_tag(obj)), export_as_value_ptr(get_inner_obj(obj)));

  OBJ_TYPE physical_type = get_physical_type(obj);
  switch (physical_type) {
    case TYPE_SYMBOL:
      return new SymbValue(symb_to_raw_str(obj));

    case TYPE_INTEGER:
      return new IntValue(get_int(obj));

    case TYPE_FLOAT:
      return new FloatValue(get_float(obj));

    case TYPE_SEQUENCE:
    case TYPE_SLICE:
      if (!is_empty_seq(obj)) {
        uint32 size = get_seq_length(obj);
        OBJ *objs = get_seq_buffer_ptr(obj);
        Value **items = new Value *[size];
        for (uint32 i=0 ; i < size ; i++)
          items[i] = export_as_value_ptr(objs[i]);
        return new SeqSetValue(items, size, true);
      }
      else
        return new SeqSetValue(NULL, 0, true);

    case TYPE_SET:
      if (!is_empty_rel(obj)) {
        SET_OBJ *ptr = get_set_ptr(obj);
        uint32 size = ptr->size;
        OBJ *objs = ptr->buffer;
        Value **items = new Value*[size];
        for (uint32 i=0 ; i < size ; i++)
          items[i] = export_as_value_ptr(objs[i]);
        return new SeqSetValue(items, size, false);
      }
      else
        return new SeqSetValue(NULL, 0, false);

    case TYPE_BIN_REL:
    case TYPE_MAP:
    case TYPE_LOG_MAP: {
      assert(!is_empty_rel(obj));
      BIN_REL_OBJ *ptr = get_bin_rel_ptr(obj);
      uint32 size = ptr->size;
      OBJ *buffer = ptr->buffer;
      Value *(*entries)[2] = new Value*[size][2];
      for (uint32 i=0 ; i < size ; i++) {
        entries[i][0] = export_as_value_ptr(buffer[i]);
        entries[i][1] = export_as_value_ptr(buffer[i+size]);
      }
      return new BinRelValue(entries, size, physical_type != TYPE_BIN_REL);
    }

    case TYPE_TERN_REL: {
      assert(!is_empty_rel(obj));
      TERN_REL_OBJ *ptr = get_tern_rel_ptr(obj);
      uint32 size = ptr->size;
      OBJ *buffer = ptr->buffer;
      Value *(*entries)[3] = new Value*[size][3];
      for (uint32 i=0 ; i < size ; i++) {
        entries[i][0] = export_as_value_ptr(buffer[3*i]);
        entries[i][1] = export_as_value_ptr(buffer[3*i+1]);
        entries[i][2] = export_as_value_ptr(buffer[3*i+2]);
      }
      return new TernRelValue(entries, size);
    }

    default: // case TYPE_BLANK_OBJ: case TYPE_NULL_OBJ: case TYPE_TAG_OBJ:
      fail();
  }
}

unique_ptr<Value> export_as_value(OBJ obj) {
  return unique_ptr<Value>(export_as_value_ptr(obj));
}
