#pragma once
// lang::Cpp

#include <cstdlib>
#include <cstring>
#include <iostream>

#define coutp(x, r)                                                            \
  std::cout << x;                                                              \
  return r
#define coutpln(x, r)                                                          \
  std::cout << x << std::endl;                                                 \
  return r

/** Helper class providing some C++ functionality and convenience
 *  functions. This class has no data, constructors, destructors or
 *  virtual functions. Inheriting from it is zero cost.
 */
class Sys {
public:
  // Printing functions
  Sys &p(char *c) { coutp(c, *this); }
  Sys &p(bool c) { coutp(c, *this); }
  Sys &p(float c) { coutp(c, *this); }
  Sys &p(int i) { coutp(i, *this); }
  Sys &p(size_t i) { coutp(i, *this); }
  Sys &p(const char *c) { coutp(c, *this); }
  Sys &p(char c) { coutp(c, *this); }
  Sys &pln() { coutp(std::endl, *this); }
  Sys &pln(int i) { coutpln(i, *this); }
  Sys &pln(char *c) { coutpln(c, *this); }
  Sys &pln(bool c) { coutpln(c, *this); }
  Sys &pln(char c) { coutpln(c, *this); }
  Sys &pln(float x) { coutpln(x, *this); }
  Sys &pln(size_t x) { coutpln(x, *this); }
  Sys &pln(const char *c) { coutpln(c, *this); }

  // Copying strings
  char *duplicate(const char *s) {
    char *res = new char[strlen(s) + 1];
    strcpy(res, s);
    return res;
  }
};
