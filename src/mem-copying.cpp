#include "lib.h"


SEQ_OBJ *make_or_get_seq_obj_copy(SEQ_OBJ *seq) {
  if (seq->capacity > 0) {
    // The object has not been copied yet. Making a new object large enough
    // to accomodate all the elements of the original sequence. We are using
    // <size> and not <capacity> for the new sequence because it's best to
    // leave the decision of how much extra memory to allocate to the memory
    // allocator, which is aware of the context.
    //## IF THIS IS THE ONLY REFERENCE TO THE SEQUENCE, AND IF <length> IS
    //## LOWER THAN SIZE, WE COULD COPY ONLY THE FIRST LENGTH ELEMENTS...
    uint32 size = seq->size;
    SEQ_OBJ *seq_copy = new_seq(size);
    // Now we copy all the elements of the sequence
    OBJ *buff = seq->buffer;
    OBJ *buff_copy = seq_copy->buffer;
    for (int i=0 ; i < size ; i++)
      buff_copy[i] = copy_obj(buff[i]);
    // We mark the old sequence as "copied", and we store a pointer to the copy
    // into it. The fields of the original object are never going to be used again,
    // even by the memory manager, so we can safely overwrite them.
    seq->capacity = 0;
    * (SEQ_OBJ **) buff = seq_copy;
    // Returning the new object
    return seq_copy;
  }
  else {
    // The object has already been copied. We just return a (reference-counted) pointer to the copy
    SEQ_OBJ *seq_copy = * (SEQ_OBJ **) seq->buffer;
    add_ref((REF_OBJ *) seq_copy);
    return seq_copy;
  }
}

SET_OBJ *make_or_get_set_obj_copy(SET_OBJ *set) {
  uint32 size = set->size;
  if (size > 0) {
    // The object has not been copied yet, so we do it now.
    SET_OBJ *set_copy = new_set(size);
    // Now we copy all the elements of the sequence
    OBJ *buff = set->buffer;
    OBJ *buff_copy = set_copy->buffer;
    for (int i=0 ; i < size ; i++)
      buff_copy[i] = copy_obj(buff[i]);
    // We mark the old sequence as "copied", and we store a pointer to the copy
    // into it. The fields of the original object are never going to be used again,
    // even by the memory manager, so we can safely overwrite them.
    set->size = 0;
    * (SET_OBJ **) buff = set_copy;
    // Returning the new object
    return set_copy;
  }
  else {
    // The object has already been copied. We just return a (reference-counted) pointer to the copy
    SET_OBJ *set_copy = * (SET_OBJ **) set->buffer;
    add_ref((REF_OBJ *) set_copy);
    return set_copy;
  }
}

BIN_REL_OBJ *make_or_get_bin_rel_obj_copy(BIN_REL_OBJ *rel) {
  uint32 size = rel->size;
  if (size > 0) {
    // The object has not been copied yet, so we do it now.
    BIN_REL_OBJ *rel_copy = new_bin_rel(size);
    // Now we copy all the elements of the collection
    OBJ *buff = rel->buffer;
    OBJ *buff_copy = rel_copy->buffer;
    for (int i=0 ; i < 2 * size ; i++)
      buff_copy[i] = copy_obj(buff[i]);
    // Now we copy the extra data at the end
    uint32 *rev_idxs = get_right_to_left_indexes(rel);
    uint32 *rev_idxs_copy = get_right_to_left_indexes(rel_copy);
    memcpy(rev_idxs_copy, rev_idxs, size * sizeof(uint32));
    // We mark the old object as "copied", and we store a pointer to the copy
    // into it. The fields of the original object are never going to be used again,
    // even by the memory manager, so we can safely overwrite them.
    rel->size = 0;
    * (BIN_REL_OBJ **) buff = rel_copy;
    // Returning the new object
    return rel_copy;
  }
  else {
    // The object has already been copied. We just return a (reference-counted) pointer to the copy
    BIN_REL_OBJ *rel_copy = * (BIN_REL_OBJ **) rel->buffer;
    add_ref((REF_OBJ *) rel_copy);
    return rel_copy;
  }

}

BIN_REL_OBJ *make_or_get_map_obj_copy(BIN_REL_OBJ *map) {
  uint32 size = map->size;
  if (size > 0) {
    // The object has not been copied yet, so we do it now.
    BIN_REL_OBJ *map_copy = new_map(size);
    // Now we copy all the elements of the sequence
    OBJ *buff = map->buffer;
    OBJ *buff_copy = map_copy->buffer;
    for (int i=0 ; i < 2 * size ; i++)
      buff_copy[i] = copy_obj(buff[i]);
    // We mark the old sequence as "copied", and we store a pointer to the copy
    // into it. The fields of the original object are never going to be used again,
    // even by the memory manager, so we can safely overwrite them.
    map->size = 0;
    * (BIN_REL_OBJ **) buff = map_copy;
    // Returning the new object
    return map_copy;
  }
  else {
    // The object has already been copied. We just return a (reference-counted) pointer to the copy
    BIN_REL_OBJ *map_copy = * (BIN_REL_OBJ **) map->buffer;
    add_ref((REF_OBJ *) map_copy);
    return map_copy;
  }
}

TAG_OBJ *make_or_get_tag_obj_copy(TAG_OBJ *tag_obj) {
  if (tag_obj->unused_field == 0) {
    // The object has not been copied yet, so we do it now
    TAG_OBJ *tag_obj_copy = new_tag_obj();
    tag_obj_copy->tag_idx = tag_obj->tag_idx;
    tag_obj_copy->obj = copy_obj(tag_obj->obj);
    // We mark the old object as "copied", and we store a pointer to the copy
    // into it. The fields of the original object are never going to be used again,
    // even by the memory manager, so we can safely overwrite them.
    tag_obj->unused_field = 0xFFFF;
    * (TAG_OBJ **) &tag_obj->obj = tag_obj_copy;
    // Returning the new object
    return tag_obj_copy;
  }
  else {
    // The object has already been copied. We just return a (reference-counted) pointer to the copy
    TAG_OBJ *tag_obj_copy = * (TAG_OBJ **) &tag_obj->obj;
    add_ref((REF_OBJ *) tag_obj_copy);
    return tag_obj_copy;
  }
}

////////////////////////////////////////////////////////////////////////////////

OBJ copy_obj(OBJ obj) {
  if (is_inline_obj(obj))
    return obj;

  if (!uses_try_mem(obj)) {
    add_ref(obj);
    return obj;
  }

  assert(is_in_copying_state());

  switch (get_physical_type(obj)) {
    case TYPE_SEQUENCE: {
      SEQ_OBJ *seq_copy = make_or_get_seq_obj_copy(get_seq_ptr(obj));
      return repoint_to_std_mem_copy(obj, seq_copy->buffer);
    }

    case TYPE_SLICE: {
      SEQ_OBJ *seq_copy = make_or_get_seq_obj_copy(get_seq_ptr(obj));
      OBJ *seq_copy_buffer = seq_copy->buffer;
      return repoint_to_std_mem_copy(obj, seq_copy_buffer + get_seq_offset(obj));
    }

    case TYPE_SET: {
      SET_OBJ *set_copy = make_or_get_set_obj_copy(get_set_ptr(obj));
      return repoint_to_std_mem_copy(obj, set_copy);
    }

    case TYPE_BIN_REL: case TYPE_LOG_MAP: {
      BIN_REL_OBJ *rel_copy = make_or_get_bin_rel_obj_copy(get_bin_rel_ptr(obj));
      return repoint_to_std_mem_copy(obj, rel_copy);
    }

    case TYPE_MAP: {
      BIN_REL_OBJ *map_copy = make_or_get_map_obj_copy(get_bin_rel_ptr(obj));
      return repoint_to_std_mem_copy(obj, map_copy);
    }

    case TYPE_TAG_OBJ: {
      TAG_OBJ *tag_obj_copy = make_or_get_tag_obj_copy(get_tag_obj_ptr(obj));
      return repoint_to_std_mem_copy(obj, tag_obj_copy);
    }

    default:
      internal_fail();
  }
}
