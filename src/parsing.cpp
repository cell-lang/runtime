#include "lib.h"


enum TOKEN_TYPE {
  COMMA,
  COLON,
  SEMICOLON,
  ARROW,
  OPEN_PAR,
  CLOSE_PAR,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  INT,
  FLOAT,
  SYMBOL,
  STRING,
  WHATEVER
};

union VALUE {
  int64 integer;
  double floating;
  uint16 symb_idx;
  struct {
    const char *ptr;
    uint32 length;
  } string;
};

struct TOKEN {
  uint32 offset;
  uint32 length;
  TOKEN_TYPE type;
  VALUE value;
};

////////////////////////////////////////////////////////////////////////////////

inline int64 read_nat(const char *text, uint32 length, int64 *offset_ptr) {
  int64 start_offset = *offset_ptr;
  int64 end_offset = start_offset;
  int64 value = 0;
  char ch;
  while (end_offset < length && isdigit(ch = text[end_offset])) {
    value = 10 * value + (ch - '0');
    end_offset++;
  }
  assert(end_offset > start_offset);
  int64 count = end_offset - start_offset;
  if (count > 19) {
    *offset_ptr = -start_offset - 1;
    return -1;
  }
  else if (count == 19) {
    static const char *MAX = "9223372036854775807";
    for (int i=0 ; i < 19 ; i++) {
      ch = text[start_offset + i];
      char max_ch = MAX[i];
      if (ch > max_ch) {
        *offset_ptr = -start_offset - 1;
        return -1;
      }
      else if (ch < max_ch)
        break;
    }
  }
  *offset_ptr = end_offset;
  return value;
}


inline int64 read_number(const char *text, uint32 length, int64 offset, TOKEN *token, bool negate) {
  char ch;

  int64 i = offset;

  int64 int_value = read_nat(text, length, &i);
  if (i < 0)
    return i;

  bool is_int;
  if (i == length)
    is_int = true;
  else {
    ch = text[i];
    is_int = ch != '.' & !islower(ch);
    assert(!isdigit(ch));
  }

  if (is_int) {
    if (token != NULL) {
      token->offset = offset;
      token->length = i - offset;
      token->type = INT;
      token->value.integer = negate ? -int_value : int_value;
    }
    return i;
  }

  double float_value = int_value;
  if (ch == '.') {
    uint32 start = ++i;
    int64 dec_int_value = read_nat(text, length, &i);
    if (i < 0)
      return i;
    float_value += ((double) dec_int_value) / pow(10, i - start);
  }

  if (i < length) {
    ch = text[i];
    if (ch == 'e') {
      if (++i == length)
        return -i - 1;
      ch = text[i];

      bool neg_exp = false;
      if (ch == '-') {
        if (++i == length)
          return -i - 1;
        ch = text[i];
        neg_exp = true;
      }

      if (!isdigit(ch))
        return -i - 1;

      int64 exp_value = read_nat(text, length, &i);
      if (i < 0)
        return i;

      float_value *= pow(10, neg_exp ? -exp_value : exp_value);
    }

    if (islower(ch))
      return -i - 1;
  }

  if (token != NULL) {
    token->offset = offset;
    token->length = i - offset;
    token->type = FLOAT;
    token->value.floating = negate ? -float_value : float_value;
  }
  return i;
}


inline int64 read_symbol(const char *text, uint32 length, int64 offset, TOKEN *token) {
  int64 i = offset;
  while (++i < length) {
    char ch = text[i];
    if (ch == '_') {
      if (++i == length)
        return -i - 1;
      ch = text[i];
      if (!islower(ch) & !isdigit(ch))
        return -i - 1;
    }
    else if (!islower(ch) & !isdigit(ch))
      break;
  }

  if (token != NULL) {
    token->offset = offset;
    token->length = i - offset;
    token->type = SYMBOL;
    token->value.symb_idx = lookup_symb_idx(text+offset, i-offset);
  }
  return i;
}


inline int64 read_string(const char *text, uint32 length, int64 offset, TOKEN *token) {
  uint32 str_len = 0;
  for (int64 i=offset+1 ; i < length ; i++) {
    char ch = text[i];

    if (ch < ' ' | ch > '~')
      return -offset - 1;

    if (ch == '"') {
      if (token != NULL) {
        token->offset = offset;
        token->length = i + 1 - offset;
        token->type = STRING;
        token->value.string.ptr = text + offset + 1;
        token->value.string.length = str_len;
      }
      return i + 1;
    }

    str_len++;

    if (ch == '\\') {
      if (++i == length)
        return -i - 1;
      ch = text[i];
      if (isxdigit(ch)) {
        if (i + 3 >= length || !(isxdigit(text[i+1]) & isxdigit(text[i+2]) & isxdigit(text[i+3])))
          return -i;
        i += 3;
      }
      else if (ch != '\\' & ch != '"' & ch != 'n' & ch != 't')
        return -i;
    }
  }
  return -(length + 1);
}


int64 tokenize(const char *text, uint32 length, TOKEN *tokens) {
  bool ok;

  uint32 index = 0;
  int64 offset = 0;

  while (offset < length) {
    char ch = text[offset];

    if (isspace(ch)) {
      offset++;
      continue;
    }

    TOKEN *token = tokens != NULL ? tokens + index : NULL;
    index++;

    bool negate = false;
    if (ch == '-') {
      if (offset + 1 == length)
        return -offset - 1;

      offset++;
      ch = text[offset];

      // Arrow
      if (ch == '>') {
        if (token != NULL) {
          token->offset = offset - 1;
          token->length = 2;
          token->type = ARROW;
        }
        offset++;
        continue;
      }

      if (!isdigit(ch))
        return -offset - 2;

      negate = true;
    }

    // Integer and floating point numbers
    if (ch >= '0' && ch <= '9') {
      offset = read_number(text, length, offset, token, negate);
      if (offset < 0)
        return offset;
      else
        continue;
    }

    // Symbols
    if (ch >= 'a' && ch <= 'z') {
      offset = read_symbol(text, length, offset, token);
      if (offset < 0)
        return offset;
      else
        continue;
    }

    // Strings
    if (ch == '"') {
      offset = read_string(text, length, offset, token);
      if (offset < 0)
        return offset;
      else
        continue;
    }

    // Single character tokens
    TOKEN_TYPE type;
    switch (ch) {
      case ',':
        type = COMMA;
        break;

      case ':':
        type = COLON;
        break;

      case ';':
        type = SEMICOLON;
        break;

      case '(':
        type = OPEN_PAR;
        break;

      case ')':
        type = CLOSE_PAR;
        break;

      case '[':
        type = OPEN_BRACKET;
        break;

      case ']':
        type = CLOSE_BRACKET;
        break;

      default:
        return -offset - 1;
    }

    if (token != NULL) {
      token->offset = offset;
      token->length = 1;
      token->type = type;
    }

    offset++;
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct STATE {
  OBJ *cols[3];
  uint32 count;
  uint32 capacity;
};

static void init(STATE *state, int arity) {
  const int MIN_CAPACITY = 128;
  OBJ **cols = state->cols;
  for (int i=0 ; i < 3 ; i++)
    cols[i] = i < arity ? new_obj_array(MIN_CAPACITY) : NULL;
  state->count = 0;
  state->capacity = MIN_CAPACITY;
}

void cleanup(STATE *state, bool release) {
  uint32 count = state->count;
  for (int c=0 ; c < 3 ; c++) {
    OBJ *col = state->cols[c];
    if (col == NULL)
      break;
    if (release)
      vec_release(col, count);
    delete_obj_array(col, state->capacity);
  }
}

void store(STATE *state, OBJ *objs, int size) {
  uint32 count = state->count;
  uint32 capacity = state->capacity;
  OBJ **cols = state->cols;
  if (count >= capacity) {
    //## ARE WE SURE HERE THAT THIS NEW CAPACITY IS ALWAYS ENOUGH?
    uint32 new_capacity = 2 * capacity;
    for (int i=0 ; i < 3 && cols[i] != NULL ; i++)
      cols[i] = resize_obj_array(cols[i], capacity, new_capacity);
    state->capacity = new_capacity;
  }
  for (int i=0 ; i < size ; i++)
    cols[i][count] = objs[i];
  state->count = count + 1;
}

////////////////////////////////////////////////////////////////////////////////

typedef int64 (*parser)(TOKEN*, uint32, int64, STATE*);

int64 read_list(TOKEN *tokens, uint32 length, int64 offset, TOKEN_TYPE sep, TOKEN_TYPE term, parser parse_elem, STATE *state) {
  // Empty list
  if (offset < length && tokens[offset].type == term)
    return offset + 1;

  for ( ; ; ) {
    offset = parse_elem(tokens, length, offset, state);

    // Unexpected EOF
    if (offset >= length)
      offset = -offset - 1;

    // Parsing failed
    if (offset < 0)
      return offset;

    TOKEN_TYPE type = tokens[offset++].type;

    // One more item
    if (type == sep)
      continue;

    // Done
    if (type == term)
      return offset;

    // Done
    if (term == WHATEVER)
      return offset - 1;

    // Unexpected separator/terminator
    return -offset;
  }
}

////////////////////////////////////////////////////////////////////////////////

int64 parse_obj(TOKEN *tokens, uint32 length, int64 offset, OBJ *var);

////////////////////////////////////////////////////////////////////////////////

int64 parse_entry(TOKEN *tokens, uint32 length, int64 offset, uint32 count, TOKEN_TYPE sep, OBJ *vars) {
  uint32 read = 0;

  for (read = 0 ; read < count ; read++) {
    if (read > 0)
      if (offset < length && tokens[offset].type == sep)
        offset++;
      else
        break;
    offset = parse_obj(tokens, length, offset, vars+read);
    if (offset < 0)
      break;
  }

  if (read == count & offset < length)
    return offset;

  for (uint32 i=0 ; i < read ; i++)
    release(vars[i]);

  return offset < 0 ? offset : -offset - 1;
}

////////////////////////////////////////////////////////////////////////////////

int64 read_obj(TOKEN *tokens, uint32 length, int64 offset, STATE *state) {
  OBJ obj;
  offset = parse_obj(tokens, length, offset, &obj);
  if (offset >= 0)
    store(state, &obj, 1);
  return offset;
}

int64 read_entry(TOKEN *tokens, uint32 length, int64 offset, STATE *state, int size, TOKEN_TYPE sep) {
  OBJ entry[3];
  offset = parse_entry(tokens, length, offset, size, sep, entry);
  if (offset >= 0)
    store(state, entry, size);
  return offset;
}

int64 read_map_entry(TOKEN *tokens, uint32 length, int64 offset, STATE *state) {
  return read_entry(tokens, length, offset, state, 2, ARROW);
}

int64 read_bin_rel_entry(TOKEN *tokens, uint32 length, int64 offset, STATE *state) {
  return read_entry(tokens, length, offset, state, 2, COMMA);
}

int64 read_tern_rel_entry(TOKEN *tokens, uint32 length, int64 offset, STATE *state) {
  return read_entry(tokens, length, offset, state, 3, COMMA);
}

int64 read_rec_entry(TOKEN *tokens, uint32 length, int64 offset, STATE *state) {
  if (offset >= length || tokens[offset].type != SYMBOL)
    return -offset - 1;
  uint16 symb_idx = tokens[offset++].value.symb_idx;
  if (offset >= length || tokens[offset].type != COLON)
    return -offset - 1;
  OBJ entry[2];
  offset = parse_obj(tokens, length, offset+1, entry+1);
  if (offset < 0)
    return offset;
  entry[0] = make_symb(symb_idx);
  store(state, entry, 2);
  return offset;
}

////////////////////////////////////////////////////////////////////////////////

int64 parse_seq(TOKEN *tokens, uint32 length, int64 offset, OBJ *var) {
  STATE state;
  init(&state, 1);

  offset = read_list(tokens, length, offset+1, COMMA, CLOSE_PAR, read_obj, &state);

  if (offset >= 0)
    *var = build_seq(state.cols[0], state.count);

  cleanup(&state, offset < 0);
  return offset;
}

////////////////////////////////////////////////////////////////////////////////

bool is_record(TOKEN *tokens, uint32 length, int64 offset) {
  return offset + 2 < length && tokens[offset+1].type == SYMBOL && tokens[offset+2].type == COLON;
}

int64 parse_rec(TOKEN *tokens, uint32 length, int64 offset, OBJ *var) {
  STATE state;
  init(&state, 2);

  offset = read_list(tokens, length, offset+1, COMMA, CLOSE_PAR, read_rec_entry, &state);

  if (offset >= 0)
    *var = build_bin_rel(state.cols[0], state.cols[1], state.count);

  cleanup(&state, offset < 0);
  return offset;
}

////////////////////////////////////////////////////////////////////////////////

int64 parse_inner_obj_or_tuple(TOKEN *tokens, uint32 length, int64 offset, OBJ *var) {
  STATE state;
  init(&state, 1);

  offset = read_list(tokens, length, offset+1, COMMA, CLOSE_PAR, read_obj, &state);
  bool ok = offset >= 0 & state.count > 0;
  if (ok)
    *var = state.count == 1 ? state.cols[0][0] : build_seq(state.cols[0], state.count);

  cleanup(&state, !ok);
  return offset;
}

int64 parse_symb_or_tagged_obj(TOKEN *tokens, uint32 length, int64 offset, OBJ *var) {
  uint16 symb_idx = tokens[offset].value.symb_idx;
  if (++offset < length) {
    if (tokens[offset].type == OPEN_PAR) {
      OBJ inner_obj;
      if (is_record(tokens, length, offset))
        offset = parse_rec(tokens, length, offset, &inner_obj);
      else
        offset = parse_inner_obj_or_tuple(tokens, length, offset, &inner_obj);
      if (offset >= 0)
        *var = make_tag_obj(symb_idx, inner_obj);
      return offset;
    }
  }
  *var = make_symb(symb_idx);
  return offset;
}

////////////////////////////////////////////////////////////////////////////////

int64 parse_rel_tail(TOKEN *tokens, uint32 length, int64 offset, int size, OBJ *first_entry, bool is_map, OBJ *var) {
  STATE state;
  init(&state, size);
  store(&state, first_entry, size);

  parser entry_parser = size == 2 ?
    (is_map ? read_map_entry : read_bin_rel_entry) :
    read_tern_rel_entry;

  offset = read_list(tokens, length, offset, is_map ? COMMA : SEMICOLON, CLOSE_BRACKET, entry_parser, &state);

  if (offset >= 0)
    if (size == 2)
      *var = build_bin_rel(state.cols[0], state.cols[1], state.count);
    else
      *var = build_tern_rel(state.cols[0], state.cols[1], state.cols[2], state.count);

  cleanup(&state, offset < 0);
  return offset;
}

////////////////////////////////////////////////////////////////////////////////

int64 parse_unord_coll(TOKEN *tokens, uint32 length, int64 offset, OBJ *var) {
  if (++offset >= length)
    return -offset - 1;
  if (tokens[offset].type == CLOSE_BRACKET) {
    *var = make_empty_rel();
    return offset + 1;
  }

  STATE state;
  init(&state, 1);

  offset = read_list(tokens, length, offset, COMMA, WHATEVER, read_obj, &state);
  if (offset < 0) {
    cleanup(&state, true);
    return offset;
  }

  TOKEN_TYPE type = tokens[offset++].type;

  bool is_map = type == ARROW & state.count == 1;
  bool is_rel = type == SEMICOLON & (state.count == 2 | state.count == 3);

  if (is_map) {
    offset = read_obj(tokens, length, offset, &state);
    if (offset >= length)
      offset = -offset - 1;
    if (offset < 0) {
      cleanup(&state, true);
      return offset;
    }
    type = tokens[offset++].type;
  }
  else if (is_rel && offset < length && tokens[offset].type == CLOSE_BRACKET) {
    type = CLOSE_BRACKET;
    offset++;
  }

  if (type == CLOSE_BRACKET) {
    if (is_map | (is_rel & state.count == 2))
      *var = build_bin_rel(state.cols[0], state.cols[0] + 1, 1);
    else if (is_rel)
      *var = build_tern_rel(state.cols[0], state.cols[0] + 1, state.cols[0] + 2, 1);
    else
      *var = build_set(state.cols[0], state.count);
    cleanup(&state, false);
    return offset;
  }

  if (is_map | is_rel) {
    OBJ entry[3];
    for (int i=0 ; i < state.count ; i++)
      entry[i] = state.cols[0][i];
    cleanup(&state, false);
    return parse_rel_tail(tokens, length, offset, state.count, entry, is_map, var);
  }

  cleanup(&state, true);
  return -offset;
}

////////////////////////////////////////////////////////////////////////////////

inline char hex_digit(char ch) {
  assert(isxdigit(ch));
  return isdigit(ch) ? (ch - '0') : (tolower(ch) - 'a' + 10);
}

void parse_string(TOKEN *token, OBJ *var) {
  const char *text = token->value.string.ptr;
  uint32 length = token->value.string.length;

  if (length == 0) {
    *var = make_tag_obj(symb_idx_string, make_empty_seq());
    return;
  }

  SEQ_OBJ *raw_str = new_seq(length);
  *var = make_tag_obj(symb_idx_string, make_seq(raw_str, length));

  OBJ *buffer = raw_str->buffer;

  for (uint32 i=0 ; i < length ; i++) {
    char ch = *(text++);
    uint16 parsed_char;
    if (ch == '\\') {
      ch = *(text++);
      if (ch == '"' | ch == '\\')
        parsed_char = ch;
      else if (ch == 'n')
        parsed_char = '\n';
      else if (ch == 't')
        parsed_char = '\t';
      else {
        char hex3 = hex_digit(ch);
        char hex2 = hex_digit(*(text++));
        char hex1 = hex_digit(*(text++));
        char hex0 = hex_digit(*(text++));
        parsed_char = 16 * (16 * (16 * hex3 + hex2) + hex1) + hex0;
      }
    }
    else
      parsed_char = ch;
    buffer[i] = make_int(parsed_char);
  }
}

////////////////////////////////////////////////////////////////////////////////

// If the function is successfull, it returns the index of the next token to consume
// If it fails, it returns the location/index of the error, negated and decremented by one
int64 parse_obj(TOKEN *tokens, uint32 length, int64 offset, OBJ *var) {
  if (offset >= length)
    return -offset - 1;

  TOKEN *token = tokens + offset;

  switch (token->type) {
    case COMMA:
    case COLON:
    case SEMICOLON:
    case ARROW:
    case CLOSE_PAR:
    case CLOSE_BRACKET:
      return -offset - 1;

    case INT:
      *var = make_int(token->value.integer);
      return offset + 1;

    case FLOAT:
      *var = make_float(token->value.floating);
      return offset + 1;

    case SYMBOL:
      return parse_symb_or_tagged_obj(tokens, length, offset, var);

    case OPEN_PAR:
      if (is_record(tokens, length, offset))
        return parse_rec(tokens, length, offset, var);
      else
        return parse_seq(tokens, length, offset, var);

    case OPEN_BRACKET:
      return parse_unord_coll(tokens, length, offset, var);

    case STRING:
      parse_string(token, var);
      return offset + 1;

    default:
      internal_fail();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool parse(const char *text, uint32 size, OBJ *var, uint32 *error_offset) {
  int64 count = tokenize(text, size, NULL);
  if (count <= 0) {
    *error_offset = count < 0 ? -count - 1 : size;
    return false;
  }

  TOKEN *tokens = (TOKEN *) new_void_array(count * sizeof(TOKEN));
  tokenize(text, size, tokens);

  memset(var, 0, sizeof(OBJ));
  int64 res = parse_obj(tokens, count, 0, var);
  if (res < 0 | res < count) {
    *error_offset = res < 0 ? tokens[-res-1].offset : size;
    delete_void_array(tokens, count * sizeof(TOKEN));
    return false;
  }

  delete_void_array(tokens, count * sizeof(TOKEN));
  return true;
}
