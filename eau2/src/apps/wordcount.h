#pragma once
// lang: CwC

#include "../client/application.h"

/** Represents a file reader that reads 1 string at a time.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class FileReader : public Writer {
public:
  /** Reads next word and stores it in the row. Actually read the word.
      While reading the word, we may have to re-fill the buffer  */
  bool accept(Row &r) override {
    assert(i_ < end_);
    assert(!isspace(buf_[i_]));
    size_t wStart = i_;
    while (true) {
      if (i_ == end_) {
        if (feof(file_)) {
          ++i_;
          break;
        }
        i_ = wStart;
        wStart = 0;
        fillBuffer_();
      }
      if (isspace(buf_[i_]))
        break;
      ++i_;
    }
    buf_[i_] = 0;
    r.set(0, new String(buf_ + wStart, i_ - wStart));
    ++i_;
    skipWhitespace_();
    return false;
  }

  /** Returns true when there are no more words to read.  There is nothing
     more to read if we are at the end of the buffer and the file has
     all been read.     */
  bool done() override { return (i_ >= end_) && feof(file_); }

  /** Creates the reader and opens the file for reading.  */
  FileReader() {
    file_ = fopen(arg.file, "r");
    assert(file_ != nullptr && "Could not open file");
    buf_ = new char[BUFSIZE + 1]; //  null terminator
    fillBuffer_();
    skipWhitespace_();
  }

  static const size_t BUFSIZE = 1024;

  /** Reads more data from the file. */
  void fillBuffer_() {
    size_t start = 0;
    // compact unprocessed stream
    if (i_ != end_) {
      start = end_ - i_;
      memcpy(buf_, buf_ + i_, start);
    }
    // read more contents
    end_ = start + fread(buf_ + start, sizeof(char), BUFSIZE - start, file_);
    i_ = start;
  }

  /** Skips spaces.  Note that this may need to fill the buffer if the
      last character of the buffer is space itself.  */
  void skipWhitespace_() {
    while (true) {
      if (i_ == end_) {
        if (feof(file_))
          return;
        fillBuffer_();
      }
      // if the current character is not whitespace, we are done
      if (!isspace(buf_[i_]))
        return;
      // otherwise skip it
      ++i_;
    }
  }

  char *buf_;
  size_t end_ = 0;
  size_t i_ = 0;
  FILE *file_;
};

/****************************************************************************/
class Adder : public Rower {
public:
  SIMap &map_;
  size_t num_words_seen = 0;

  Adder(SIMap &map) : map_(map) {}

  bool accept(Row &r) {
    String *word = r.get_string(0);
    num_words_seen++;
    assert(word != nullptr);
    int sum = map_.contains_key(word) ? map_.get(word) : 0;
    map_.put(word, sum + 1);
    return false;
  }
};

/***************************************************************************/
class Summer : public Writer {
public:
  StringArray words_;
  IntArray counts_;
  size_t index_ = 0;

  Summer(SIMap &map) : words_(*map.keys()), counts_(*map.values()) {}

  void next() { index_++; }

  String *k() { return words_.get(index_); }
  int v() { return counts_.get(index_); }

  bool accept(Row &r) {
    r.set(0, k()->clone());
    r.set(1, v());
    next();
    return false;
  }

  bool done() { return index_ >= words_.size(); }
};

/****************************************************************************
 * Calculate a word count for given file:
 *   1) read the data (single node)
 *   2) produce word counts per homed chunks, in parallel
 *   3) combine the results
 **********************************************************author: pmaj ****/
class WordCount : public Application {
public:
  static const size_t BUFSIZE = 1024;
  Key data;
  SIMap all;

  WordCount(size_t idx, Network *net)
      : Application(idx, net), data("data", 0) {}

  /** The master nodes reads the input, then all of the nodes count. */
  void run_() override {
    if (this_node() == 0) {
      FileReader fr;
      DataFrame *df = DataFrame::fromVisitor(&data, this_store(), "S", fr);

      std::cout << "dataframe length (num words) " << df->dist_scm_->length()
                << std::endl;

      delete df;
    }

    local_count();
    reduce();

    if (this_node() == 0) {
      stop_all();
    }
  }

  /** Returns a key for given node. **/
  Key *mk_key(size_t idx) {
    KeyBuff kb(new Key("wc-map-", idx));
    Key *k = kb.c(idx).get();
    p("Created key ").pln(k->key()->c_str());
    return k;
  }

  /** Compute word counts on the local node and build a data frame. */
  void local_count() {
    DataFrame *words = this_store()->get_and_wait(&data);
    p("Node ").p(this_node()).pln(": starting local count...");
    SIMap map;
    Adder add(map);
    words->local_map(add);
    delete words;

    Summer cnt(map);
    Key *wc_key = mk_key(this_node());
    delete DataFrame::fromVisitor(wc_key, this_store(), "SI", cnt);
    delete wc_key;

    p("Node ").p(this_node()).p(" saw ").p(map.size()).pln(" distinct words.");
    IntArray *totals = map.values();
    int total = 0;
    for (size_t i = 0; i < totals->size(); i++) {
      total += totals->get(i);
    }
    p("Node ").p(this_node()).p(" saw ").p(total).pln(" total words.");
  }

  /** Merge the data frames of all nodes */
  void reduce() {
    if (this_node() != 0)
      return;
    pln("Node 0: reducing counts...");
    SIMap map;
    Key *own = mk_key(0);
    merge(this_store()->get(own), map);
    for (size_t i = 1; i < arg.num_nodes; ++i) { // merge other nodes
      Key *ok = mk_key(i);
      merge(this_store()->get_and_wait(ok), map);
      delete ok;
    }
    p("Total distinct words: ").pln(map.size());
    delete own;
  }

  void merge(DataFrame *df, SIMap &m) {
    Adder add(m);
    df->distributed_map(add);
    delete df;
  }
};