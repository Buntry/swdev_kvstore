#pragma once
// lang:CwC

#include "object.h"

static const double DOUBLE_TOLERANCE = 0.0001;

class Util : public Object {
public:
  /** Equality functions **/
  static bool equals(Object *a, Object *b) {
    if (a == nullptr && b == nullptr) {
      return true;
    } else if (a == nullptr && b != nullptr) {
      return false;
    } else if (a != nullptr && b == nullptr) {
      return false;
    } else {
      return a->equals(b);
    }
  }

  static bool equals(int a, int b) { return a == b; }
  static bool equals(float a, float b) { return a == b; }
  static bool equals(double a, double b) {
    return !((a - b > DOUBLE_TOLERANCE) || (b - a > DOUBLE_TOLERANCE));
  }
  static bool equals(bool a, bool b) { return a == b; }

  /** Hash functions **/
  static size_t hash(Object *a) { return (a == nullptr) ? 0 : a->hash(); }
  static size_t hash(int a) { return a; }
  static size_t hash(float a) { return std::hash<float>{}(a); }   // lang: Cpp
  static size_t hash(double a) { return std::hash<double>{}(a); } // lang: Cpp
  static size_t hash(bool a) { return a; }

  /** Clone functions **/
  static Object *clone(Object *a) { return (a == nullptr) ? a : a->clone(); }
  static int clone(int a) { return a; }
  static float clone(float a) { return a; }
  static double clone(double a) { return a; }
  static bool clone(bool a) { return a; }
};