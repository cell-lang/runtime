#include <string>
#include <ostream>


namespace cell {
  class Value {
  public:
    virtual bool is_symb() = 0;
    virtual bool is_int() = 0;
    virtual bool is_float() = 0;
    virtual bool is_seq() = 0;
    virtual bool is_set() = 0;
    virtual bool is_bin_rel() = 0;
    virtual bool is_tern_rel() = 0;
    virtual bool is_tagged() = 0;

    virtual const char *as_symb() = 0;
    virtual long long as_int() = 0;
    virtual double as_float() = 0;

    virtual unsigned int size() = 0;
    virtual Value *item(unsigned int) = 0;
    virtual void entry(unsigned int, Value *&, Value *&) = 0;
    virtual void entry(unsigned int, Value *&, Value *&, Value *&) = 0;

    virtual const char *tag() = 0;
    virtual Value *untagged() = 0;

    virtual bool is_string() = 0;
    virtual bool is_record() = 0;

    virtual std::string as_str() = 0;
    virtual Value *lookup(const char *) = 0;

    virtual std::string printed() = 0;
    virtual void print(std::ostream &os) = 0;
  };
}
