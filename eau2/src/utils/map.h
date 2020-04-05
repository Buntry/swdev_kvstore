#pragma once

#include "array.h"

// The default load for our Map implementation.
static const float MIN_LOAD_FACTOR = 0.6;

#define generate_classmapnode(KlassMapNode, Key, Value)                        \
  class KlassMapNode : public Object {                                         \
  public:                                                                      \
    KlassMapNode *next = nullptr;                                              \
    Key key;                                                                   \
    Value value;                                                               \
                                                                               \
    KlassMapNode(Key k, Value v) : key((Key)Util::clone(k)), value(v) {}       \
                                                                               \
    ~KlassMapNode() {                                                          \
      Util::destroy(key);                                                      \
      delete next;                                                             \
    }                                                                          \
  }

#define generate_classmap(KlassMap, KlassNode, KArray, VArray, K, V)           \
  generate_classmapnode(KlassNode, K, V);                                      \
  class KlassMap : public Object {                                             \
  public:                                                                      \
    size_t num_elements_ = 0;                                                  \
    size_t num_nodes_ = CHUNK_SIZE;                                            \
    KlassNode **nodes_ = nullptr;                                              \
                                                                               \
    KlassMap() {                                                               \
      nodes_ = new KlassNode *[num_nodes_];                                    \
      for (size_t i = 0; i < num_nodes_; i++) {                                \
        nodes_[i] = nullptr;                                                   \
      }                                                                        \
    }                                                                          \
                                                                               \
    virtual ~KlassMap() {                                                      \
      for (size_t i = 0; i < num_nodes_; i++) {                                \
        delete nodes_[i];                                                      \
      }                                                                        \
      delete[] nodes_;                                                         \
    }                                                                          \
                                                                               \
    V get(K k) {                                                               \
      assert(contains_key(k));                                                 \
      KlassNode *node = node_from_key_(k);                                     \
      return node->value;                                                      \
    }                                                                          \
                                                                               \
    void put(K k, V v) {                                                       \
      if (contains_key(k)) {                                                   \
        KlassNode *kn = node_from_key_(k);                                     \
        kn->value = v;                                                         \
      } else {                                                                 \
        size_t i = Util::hash(k) % num_nodes_;                                 \
        KlassNode *to_insert = new KlassNode(k, v);                            \
        to_insert->next = nodes_[i];                                           \
        nodes_[i] = to_insert;                                                 \
        num_elements_++;                                                       \
        check_load_factor_();                                                  \
      }                                                                        \
    }                                                                          \
                                                                               \
    bool contains_key(K k) { return node_from_key_(k) != nullptr; }            \
                                                                               \
    V remove(K k) {                                                            \
      assert(contains_key(k));                                                 \
      size_t i = (Util::hash(k) % num_nodes_);                                 \
      KlassNode *prev = nullptr, *cur = nodes_[i];                             \
      while (cur != nullptr && !Util::equals(k, cur->key)) {                   \
        prev = cur;                                                            \
        cur = cur->next;                                                       \
      }                                                                        \
      V ret = cur->value;                                                      \
      if (prev == nullptr) {                                                   \
        nodes_[i] = cur->next;                                                 \
      } else {                                                                 \
        prev->next = cur->next;                                                \
      }                                                                        \
      cur->next = nullptr;                                                     \
      delete cur;                                                              \
      num_elements_--;                                                         \
      return ret;                                                              \
    }                                                                          \
                                                                               \
    KArray *keys() {                                                           \
      KArray *ka = new KArray();                                               \
      for (size_t i = 0; i < num_nodes_; i++) {                                \
        for (KlassNode *cur = nodes_[i]; cur != nullptr; cur = cur->next) {    \
          ka->push_back(cur->key);                                             \
        }                                                                      \
      }                                                                        \
      return ka;                                                               \
    }                                                                          \
                                                                               \
    VArray *values() {                                                         \
      VArray *va = new VArray();                                               \
      for (size_t i = 0; i < num_nodes_; i++) {                                \
        for (KlassNode *cur = nodes_[i]; cur != nullptr; cur = cur->next) {    \
          va->push_back(cur->value);                                           \
        }                                                                      \
      }                                                                        \
      return va;                                                               \
    }                                                                          \
                                                                               \
    size_t size() { return num_elements_; }                                    \
                                                                               \
    KlassNode *node_from_key_(K k) {                                           \
      size_t i = Util::hash(k) % num_nodes_;                                   \
      KlassNode *node = nodes_[i];                                             \
      while (node != nullptr && !Util::equals(node->key, k)) {                 \
        node = node->next;                                                     \
      }                                                                        \
      return node;                                                             \
    }                                                                          \
                                                                               \
    void check_load_factor_() {                                                \
      if (num_elements_ > MIN_LOAD_FACTOR * num_nodes_) {                      \
        rehash_(num_nodes_ * 2);                                               \
      }                                                                        \
    }                                                                          \
                                                                               \
    void rehash_(size_t new_num_nodes) {                                       \
      KlassNode **old_nodes = nodes_;                                          \
      size_t old_num_nodes = num_nodes_;                                       \
      nodes_ = new KlassNode *[new_num_nodes];                                 \
      for (size_t i = 0; i < new_num_nodes; i++) {                             \
        nodes_[i] = nullptr;                                                   \
      }                                                                        \
      num_nodes_ = new_num_nodes;                                              \
      num_elements_ = 0;                                                       \
      for (size_t i = 0; i < old_num_nodes; i++) {                             \
        for (KlassNode *cur = old_nodes[i]; cur != nullptr; cur = cur->next) { \
          put(cur->key, cur->value);                                           \
        }                                                                      \
        delete old_nodes[i];                                                   \
      }                                                                        \
      delete[] old_nodes;                                                      \
    }                                                                          \
  }

/** Generates a Mapping from Object to Object **/
generate_classmap(Map, MapNode, Array, Array, Object *, Object *);

/** Represents a Mapping from String* to int **/
generate_classmap(SIMap, SINode, StringArray, IntArray, String *, int);

#define generate_object_classmap(KlassMap, KlassArray, Klass)                  \
  class KlassMap : public Map {                                                \
  public:                                                                      \
    Klass *get(Object *key) { return dynamic_cast<Klass *>(Map::get(key)); }   \
  }
