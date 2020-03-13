// lang: CwC
#pragma once

#include "string.h"

/** Advance the buffer by the given amount. **/
void advance(char *&bytes, size_t amt) {
  for (size_t i = 0; i < amt; i++) {
    bytes++;
  }
}

/** Pack doubles into a buffer. **/
void packd(char *&bytes, double d) {
  memcpy(bytes, &d, sizeof(double));
  advance(bytes, sizeof(double));
}

/** Unpack a double from a buffer. **/
double unpackd(char *&bytes) {
  double d;
  memcpy(&d, bytes, sizeof(double));
  advance(bytes, sizeof(double));
  return d;
}

/** Pack an integer from a buffer. **/
void packi(char *&bytes, int i) {
  memcpy(bytes, &i, sizeof(int));
  advance(bytes, sizeof(int));
}

/** Unpack an integer from a buffer. **/
int unpacki(char *&bytes) {
  int i;
  memcpy(&i, bytes, sizeof(int));
  advance(bytes, sizeof(int));
  return i;
}

/** Pack a size_t into a buffer. **/
void packst(char *&bytes, size_t st) {
  memcpy(bytes, &st, sizeof(size_t));
  advance(bytes, sizeof(size_t));
}

/** Unpack a size_t from a buffer. **/
size_t unpackst(char *&bytes) {
  size_t st;
  memcpy(&st, bytes, sizeof(size_t));
  advance(bytes, sizeof(size_t));
  return st;
}

/** Pack a string into a buffer. **/
void packs(char *&bytes, String *s) {
  if (s == nullptr) {
    *bytes++ = '\0';
    return;
  }
  size_t len = s->size() + 1;
  packst(bytes, len);
  memcpy(bytes, s->c_str(), len);
  advance(bytes, len);
}

/** Unpacks a string from a buffer. **/
String *unpacks(char *&bytes) {
  if (*bytes == '\0') {
    bytes++;
    return nullptr;
  }

  size_t len = unpackst(bytes);
  char *demo = new char[len];
  memcpy(demo, bytes, len);
  advance(bytes, len);

  assert(len >= 1);
  return new String(true, demo, len - 1);
}
