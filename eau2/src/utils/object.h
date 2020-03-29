#pragma once
#include "helper.h"
#include "serializer-fd.h"
// lang: CwC

/** Base class for all objects in the system.
 *  author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu */
class Object : public Sys {
public:
  size_t hash_; // every object has a hash, subclasses must implement the
                // functionality

  /** Initially sets this object to null and sets its value kind to object. **/
  Object() { hash_ = 0; }

  /** Subclasses may have something to do on finalziation */
  virtual ~Object() {}

  /** Return the hash value of this object */
  virtual size_t hash() { return hash_ != 0 ? hash_ : hash_ = hash_me(); }

  /** Subclasses should redefine */
  virtual bool equals(Object *other) { return this == other; }

  /** Return a copy of the object; nullptr is considered an error */
  virtual Object *clone() { return nullptr; }

  /** Compute the hash code (subclass responsibility) */
  virtual size_t hash_me() { return reinterpret_cast<size_t>(this); };

  /** Returned c_str is owned by the object, don't modify nor delete. */
  virtual char *c_str() { return nullptr; }

  /** Serialize this object onto a serializer. **/
  virtual void serialize(Serializer &ser) {}
};
