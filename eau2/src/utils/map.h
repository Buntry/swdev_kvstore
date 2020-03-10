#pragma once

#include "object.h"
#include "string.h"

// The default load for our Map implementation.
static const float LOAD_FACTOR = 0.6;

/**
 * @brief Represents a node in a Map from Str to Obj.
 *
 * A MapNode has a key and value pair to build our map
 * and points to the next MapNode.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class MapNode : public Object {
 public:
  MapNode *next;
  String *key;
  Object *value;

  /**
   * @brief Construct a new Map Str Obj Node object
   *
   * By default, next is a nullptr.
   * @param akey
   * @param avalue
   */
  MapNode(String *akey, Object *avalue) : Object() {
    key = akey;
    value = avalue;
    next = nullptr;
  }

  /**
   * @brief Destroy the Map Str Obj Node object
   *
   */
  virtual ~MapNode() {
    delete key;
    delete next;
  }
};

/**
 * Represents a map where the key is a string and the value is a object.
 * Inherits from map. Example: { "string_list: new StrList("hi", "bye"),
 * "string": new String("hello") }
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class Map : public Object {
 public:
  size_t num_elements_;
  size_t num_nodes_;
  MapNode **nodes_;

  /**
   * @brief Construct a new Map Str Obj object
   *
   * We initialize a MapStrObj to have 10 nodes and no elements.
   * These nodes all have nullptrs.
   */
  Map() : Object() {
    num_nodes_ = 10;
    num_elements_ = 0;
    nodes_ = new MapNode *[num_nodes_];

    for (size_t i = 0; i < num_nodes_; i++) {
      nodes_[i] = nullptr;
    }
  }

  /**
   * @brief Destroy the Map Str Obj object
   *
   */
  virtual ~Map() {
    for (size_t i = 0; i < num_nodes_; i++) {
      delete nodes_[i];
    }
    delete[] nodes_;
  }

  /**
   * @brief Resize the nodes of this Map object to
   * the new number of nodes given.
   *
   * These new nodes must be initialized to the new
   * size. All the elements in the current Map are reinserted
   * into the new nodes.
   *
   * @param new_num_nodes
   */
  virtual void resize_to_(size_t new_num_nodes) {
    MapNode **old_nodes = nodes_;
    size_t old_num_nodes = num_nodes_;

    // Initialize new nodes
    nodes_ = new MapNode *[new_num_nodes];
    for (size_t i = 0; i < new_num_nodes; i++) {
      nodes_[i] = nullptr;
    }
    // Decrease the size and increase the number of nodes to match.
    num_nodes_ = new_num_nodes;
    num_elements_ = 0;

    // Insert all the elements that were in this map back.
    for (size_t i = 0; i < old_num_nodes; i++) {
      MapNode *node = old_nodes[i];
      while (node != nullptr) {
        MapNode *cur = node;
        put(cur->key, cur->value);
        node = cur->next;
        cur->key = nullptr;
        cur->value = nullptr;
        cur->next = nullptr;
        delete cur;
      }
    }
  }

  /**
   * Adds a mapping from key to val to the map. A key must be unique, if there
   * is already a key of the same value in the map, the value will be
   * overwritten by the new value.
   *
   * @param key unique string key
   * @param val object value
   */
  void put(String *key, Object *val) {
    size_t index = (key->hash() % num_nodes_);
    MapNode *node = nodes_[index];
    while (node != nullptr) {
      if (node->key->equals(key)) {
        node->value = val;
        return;
      }
      node = node->next;
    }
    MapNode *to_insert = new MapNode(key, val);
    to_insert->next = nodes_[index];
    nodes_[index] = to_insert;
    num_elements_++;

    float load_factor = num_elements_ / (num_nodes_ + 0.0);
    if (load_factor > LOAD_FACTOR) {
      resize_to_(num_nodes_ * 2);
    }
  }

  /**
   * Returns the object that the key maps to.
   * @param key key
   * @return value key maps to
   */
  Object *get(String *key) {
    size_t index = (key->hash() % num_nodes_);
    MapNode *node = nodes_[index];
    while (node != nullptr && !node->key->equals(key)) {
      node = node->next;
    }
    return (node == nullptr) ? nullptr : node->value;
  }

  /**
   * Removes the mapping from the map.
   * @param key key of mapping to remove
   * @return value that was removed
   */
  Object *remove(String *key) {
    size_t index = (key->hash() % num_nodes_);
    MapNode *prev = nullptr;
    MapNode *node = nodes_[index];
    while (node != nullptr && !node->key->equals(key)) {
      prev = node;
      node = node->next;
    }
    if (node == nullptr) {
      return nullptr;
    }
    if (prev == nullptr) {
      nodes_[index] = node->next;
    } else {
      prev->next = node->next;
    }

    num_elements_--;
    Object *to_return = node->value;
    node->value = nullptr;
    node->key = nullptr;
    node->next = nullptr;
    delete node;
    return to_return;
  }

  /**
   * Determines if the map contains the given key.
   * @param key key
   * @return if the key is in the map
   */
  bool contains_key(String *key) {
    size_t index = (key->hash() % num_nodes_);
    MapNode *node = nodes_[index];
    while (node != nullptr && !node->key->equals(key)) {
      node = node->next;
    }
    return node != nullptr;
  }

  /**
   * Returns a list of keys in the map.
   * @return list of keys
   */
  String **get_keys() {
    String **to_return = new String *[num_elements_];
    size_t cur_index = 0;
    for (size_t i = 0; i < num_nodes_; i++) {
      MapNode *node = nodes_[i];
      while (node != nullptr) {
        to_return[cur_index] = node->key;
        cur_index++;
        node = node->next;
      }
    }
    return to_return;
  }

  /**
   * Returns a list of values in the map.
   * @return list of values
   */
  Object **get_values() {
    Object **to_return = new Object *[num_elements_];
    size_t cur_index = 0;
    for (size_t i = 0; i < num_nodes_; i++) {
      MapNode *node = nodes_[i];
      while (node != nullptr) {
        to_return[cur_index] = node->value;
        cur_index++;
        node = node->next;
      }
    }
    return to_return;
  }

  /**
   * @brief Gets the size of this Map
   * by returning the number of elements.
   *
   * @return size_t
   */
  size_t size() { return num_elements_; }
};
