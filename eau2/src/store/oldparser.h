// lang: CwC
#pragma once

#include "column.h"
#include "rows.h"

/**
 * Code was adapted from https://github.ccs.neu.edu/euhlmann/CS4500-A1-part1.
 */

/**
 * Enum representing what mode the parser is currently using for parsing.
 */
enum class MyParserMode {
  /** We're trying to find the number of columns in the schema */
  DETECT_NUM_COLS,
  /** We're guessing the column types */
  DETECT_SCHEMA,
  /** Parsing the whole file */
  PARSE_FILE
};

/**
 * Represents a slice of a larger c-style string. Lightweight replacement for
 * allocating/copying regular c-style-strings.
 */
class MyStrSlice : public Object {
public:
  size_t start_;
  size_t end_;
  const char *str_;
  /**
   * Creates a new StrSlice.
   * @param str The C-style-string to slice from. Must be valid during the
   * lifetime of this slice
   * @param start The starting index
   * @param end The ending index
   */
  MyStrSlice(const char *str, size_t start, size_t end) : Object() {
    start_ = start;
    end_ = end;
    str_ = str;
  }

  /**
   * @return The length of this slice
   */
  virtual size_t size() { return end_ - start_; }
  /**
   * @return A non-null-terminated pointer to the chars in this slice.
   */
  virtual const char *get_chars() { return &str_[start_]; }
  /**
   * Gets the char at the given index.
   * @param which The index. Must be < getLength()
   * @return The char
   */
  virtual char get_char(size_t which) {
    assert(which < end_ - start_);
    return str_[start_ + which];
  }

  /**
   * Updates the bounds of this slice to exclude instances of the given char at
   * the beginning or end. May shorten the length of this slice.
   * @param c The char to remove from the beginning and end
   */
  virtual void trim(char c) {
    while (start_ < end_ && str_[start_] == c) {
      start_++;
    }
    while (start_ < end_ && str_[end_ - 1] == c) {
      end_--;
    }
  }

  /**
   * Allocates a new null terminated string and copies the contents of this
   * slice to it.
   * @return The new string. Caller must free
   */
  virtual char *to_cstr() {
    size_t length = end_ - start_;
    char *sliceCopy = new char[length + 1];
    memcpy(sliceCopy, get_chars(), length);
    sliceCopy[length] = '\0';
    return sliceCopy;
  }

  /**
   * Parses the contents of this slice as an int.
   * @return An int corresponding to the digits in this slice.
   */
  virtual int to_int() {
    // Roll a custom integer parsing function to avoid having to allocate a new
    // null-terminated string for atoi and friends.
    long result = 0;
    bool is_negative = false;
    for (size_t i = start_; i < end_; i++) {
      char c = str_[i];
      if (i == start_ && c == '-') {
        is_negative = true;
        continue;
      } else if (i == start_ && c == '+') {
        continue;
      } else if (c >= '0' && c <= '9') {
        result = result * 10 + (c - '0');
      } else {
        break;
      }
    }
    return is_negative ? -result : result;
  }

  /**
   * Parses the contents of this slice as a float.
   * @return The float
   */
  virtual float to_float() {
    // It's hard to roll a float parsing function by hand, so bite the bullet
    // and allocate a null-terminated string for atof.
    char *cstr = to_cstr();
    float result = atof(cstr);
    delete[] cstr;
    return result;
  }

  /** **
   * Parses the contents of thsi slice as a string.
   * @return the string.
   **/
  virtual String *to_str() { return new String(true, to_cstr(), size()); }
};

/** This parser is able to parse the entirety of Schema-on-read files **/
class SorParser : public Object {
public:
  Schema scm_;
  FILE *file_ = nullptr;
  size_t start_ = 0;
  size_t end_ = 0;

  char *line = nullptr;
  size_t len = 0;
  size_t bytes = 0;
  Row *working_row_ = nullptr;
  ColumnArray cols;

  static const size_t SCM_LINES = 500;
  static const char FIELD_BEGIN = '<';
  static const char FIELD_END = '>';
  static const char STRING_QUOTE = '"';
  static const char SPACE = ' ';
  static const char DOT = '.';
  static const char PLUS = '+';
  static const char MINUS = '-';

  SorParser(const char *fn) {
    assert((file_ = fopen(fn, "r")) != nullptr);
    fseek(file_, 0, SEEK_END);
    end_ = ftell(file_);
    fseek(file_, 0, SEEK_SET);
  }

  ~SorParser() {
    delete working_row_;
    free(line);
    fclose(file_);
  }

  /** Infers the schema of the file by looking at the first 500 lines. **/
  void infer_schema() {
    CharArray cols;

    // First get the number of columns
    size_t max = 0;
    for (size_t i = 0;
         i < SCM_LINES && (bytes = getline(&line, &len, file_)) != -1UL; i++) {
      size_t cur = scan_line_(line, bytes, cols, working_row(),
                              MyParserMode::DETECT_NUM_COLS);
      max = (max > cur) ? max : cur;
    }
    rewind(file_);

    // Assume all of them are booleans
    for (size_t i = 0; i < max; i++)
      cols.push_back('B');

    // Guess the schema
    for (size_t i = 0;
         i < SCM_LINES && (bytes = getline(&line, &len, file_)) != -1UL; i++)
      scan_line_(line, bytes, cols, working_row(), MyParserMode::DETECT_SCHEMA);

    for (size_t i = 0; i < cols.size(); i++)
      scm_.add_column(cols.get(i));

    working_row_ = new Row(scm_);
    rewind(file_);
  }

  /** Parses the next line and returns whether it set the working row. This
   * function assumes that the schema has been inferred prior. **/
  bool parse_line(Row *row) {
    if ((bytes = getline(&line, &len, file_)) != -1UL) {
      scan_line_(line, bytes, *scm_.types(), row, MyParserMode::PARSE_FILE);
      start_ += bytes;
      return true;
    }
    return false;
  }
  bool parse_line() { return parse_line(working_row()); }

  /** Determines if whether the end of the file has been reached. **/
  bool done() { return start_ >= end_; }

  /** Returns a read-only copy of this schema **/
  Schema *get_schema() { return &scm_; }

  /** Scans one line of the file and defers to another function depending
   * on the current mode of the parser. **/
  size_t scan_line_(char *line, size_t len, CharArray &cols, Row *row,
                    MyParserMode mode) {
    size_t num_fields = 0;
    size_t field_start = 0;
    bool in_field = false;
    bool in_string = false;

    // Iterate over the line, create slices for each detected field, and call
    // either _guessFieldType for ParserMode::DETECT_SCHEMA or _appendField for
    // ParserMode::PARSE_FILE for ParserMode::DETECT_NUM_COLUMNS we simply
    // return the number of fields we saw
    for (size_t i = 0; i < len; i++) {
      char c = line[i];
      if (!in_field) {
        if (c == FIELD_BEGIN) {
          in_field = true;
          field_start = i;
        }
      } else {
        if (c == STRING_QUOTE) {
          // Allow > inside quoted strings
          in_string = !in_string;
        } else if (c == FIELD_END && !in_string) {
          if (mode == MyParserMode::DETECT_SCHEMA) {
            guess_field_type_(MyStrSlice(line, field_start + 1, i), num_fields,
                              cols);
          } else if (mode == MyParserMode::PARSE_FILE) {
            append_field_(MyStrSlice(line, field_start + 1, i), num_fields, row,
                          cols);
          }
          in_field = false;
          num_fields++;
        }
      }
    }
    return num_fields;
  }

  /** Guesses the type of field within the boundary of the char* **/
  void guess_field_type_(MyStrSlice slice, size_t col, CharArray &cols) {
    slice.trim(SPACE);
    if (slice.size() == 0) {
      return;
    }

    // Check if the slice consists of only numeric chars
    // and specifically whether it has a . (indicating float)
    bool is_numeric = false;
    bool has_dot = false;
    for (size_t i = 0; i < slice.size(); i++) {
      char c = slice.get_char(i);
      if (numeric(c)) {
        is_numeric = true;
        if (c == DOT) {
          has_dot = true;
        }
      }
    }

    char type = cols.get(col);
    // If the guess is already string, we can't change that because that means
    // we have seen a non-numeric entry already
    if (type != 'S') {
      if (is_numeric && !has_dot) {
        // If it's an integer (not float), check if it's 0 or 1, which would
        // indicate a bool column
        int val = slice.to_int();
        if ((val == 0 || val == 1) && type != 'I' && type != 'F') {
          // Only keep the bool column guess if we haven't already guessed
          // integer or float (because that means we have seen non-bool values)
          cols.set(col, 'B');
        } else if (type != 'F') {
          // Use integer guess only if we didn't already guess float (which
          // could not be parsed as integer)
          cols.set(col, 'I');
        }
      } else if (is_numeric && has_dot) {
        // If there's a dot, this must be a float column
        cols.set(col, 'F');
      } else {
        // If there are non-numeric chars then this must be a string column
        cols.set(col, 'S');
      }
    }
  }

  /** Appends the field to the row  **/
  void append_field_(MyStrSlice slice, size_t col, Row *row, CharArray &cols) {
    slice.trim(SPACE);
    if (slice.size() == 0) {
      row->set_missing(col);
      return;
    }

    switch (cols.get(col)) {
    case 'S':
      slice.trim(STRING_QUOTE);
      row->set(col, slice.to_str());
      break;
    case 'I':
      row->set(col, slice.to_int());
      break;
    case 'F':
      row->set(col, slice.to_float());
      break;
    case 'B':
      row->set(col, slice.to_int() == 1);
      break;
    default:
      assert(false);
    }
  }

  /** Checks if the given char could be part of an integer or float field */
  static bool numeric(char c) {
    return c == MINUS || c == PLUS || c == DOT || (c >= '0' && c <= '9');
  }

  /** Gets the working row. **/
  Row *working_row() { return working_row_; }
};

/** Writes a Sor to a dataframe. **/
class SorWriter : public Writer {
public:
  Schema scm_;
  SorParser sor_;

  /** Writes the entirety of a sor file to a dataframe **/
  SorWriter(const char *fn) : sor_(fn) { sor_.infer_schema(); }

  /** Gets the inferred schema of the file (read-only). **/
  Schema *get_schema() { return sor_.get_schema(); }

  /** Writes a row (assumes the same schema as get_schema) **/
  bool accept(Row &row) { return sor_.parse_line(&row); }
  bool done() { return sor_.done(); }
};