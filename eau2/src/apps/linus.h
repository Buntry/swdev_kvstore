#pragma once
// lang: CwC

#include "../client/application.h"

/**
 * The input data is a processed extract from GitHub.
 *
 * projects:  I x S   --  The first field is a project id (or pid).
 *                    --  The second field is that project's name.
 *                    --  In a well-formed dataset the largest pid
 *                    --  is equal to the number of projects.
 *
 * users:    I x S    -- The first field is a user id, (or uid).
 *                    -- The second field is that user's name.
 *
 * commits: I x I x I -- The fields are pid, uid, uid', each row represent
 *                    -- a commit to project pid, written by user uid
 *                    -- and committed by user uid',
 **/

/**************************************************************************
 * A bit set contains size() booleans that are initialize to false and can
 * be set to true with the set() method. The test() method returns the
 * value. Does not grow.
 ************************************************************************/
class Set {
public:
  bool *vals_;  // owned; data
  size_t size_; // number of elements

  /** Creates a set of the same size as the dataframe. */
  Set(DataFrame *df) : Set(df->nrows()) {}

  /** Creates a set of the given size. */
  Set(size_t sz) : vals_(new bool[sz]), size_(sz) {
    for (size_t i = 0; i < size_; i++)
      vals_[i] = false;
  }

  ~Set() { delete[] vals_; }

  /** Add idx to the set. If idx is out of bound, ignore it.  Out of bound
   *  values can occur if there are references to pids or uids in commits
   *  that did not appear in projects or users.
   */
  void set(size_t idx) {
    if (idx >= size_)
      return; // ignoring out of bound writes
    vals_[idx] = true;
  }

  /** Is idx in the set?  See comment for set(). */
  bool test(size_t idx) {
    if (idx >= size_)
      return true; // ignoring out of bound reads
    return vals_[idx];
  }

  size_t size() { return size_; }

  /** Performs set union in place. */
  void union_(Set &from) {
    for (size_t i = 0; i < from.size_; i++)
      if (from.test(i))
        set(i);
  }
};

/*******************************************************************************
 * A SetUpdater is a reader that gets the first column of the data frame and
 * sets the corresponding value in the given set.
 ******************************************************************************/
class SetUpdater : public Rower {
public:
  Set &set_; // set to update

  SetUpdater(Set &set) : set_(set) {}

  /** Assume a row with at least one column of type I. Assumes that there
   * are no missing. Reads the value and sets the corresponding position.
   * The return value is irrelevant here. */
  bool accept(Row &row) {
    set_.set(row.get_int(0));
    return false;
  }
};

/*****************************************************************************
 * A SetWriter copies all the values present in the set into a one-column
 * dataframe. The data contains all the values in the set. The dataframe has
 * at least one integer column.
 ****************************************************************************/
class SetWriter : public Writer {
public:
  Set &set_;      // set to read from
  size_t id_ = 0; // position in set

  SetWriter(Set &set) : set_(set) {}

  /** Stop when the entire set has been seen */
  bool done() {
    while (id_ < set_.size_ && set_.test(id_) == false)
      id_++;
    return id_ >= set_.size();
  }

  /** Writes the next truthy idx **/
  bool accept(Row &row) { row.set(0, (int)id_++); }
};

/***************************************************************************
 * The ProjectTagger is a reader that is mapped over commits, and marks all
 * of the projects to which a collaborator of Linus committed as an author.
 * The commit dataframe has the form:
 *    pid x uid x uid
 * where the pid is the identifier of a project and the uids are the
 * identifiers of the author and committer. If the author is a collaborator
 * of Linus, then the project is added to the set. If the project was
 * already tagged then it is not added to the set of newProjects.
 *************************************************************************/
class ProjectsTagger : public Rower {
public:
  Set &uSet;       // set of collaborator
  Set &pSet;       // set of projects of collaborators
  Set newProjects; // newly tagged collaborator projects

  ProjectsTagger(Set &uSet, Set &pSet, DataFrame *proj)
      : uSet(uSet), pSet(pSet), newProjects(proj) {}

  /** The data frame must have at least two integer columns. The newProject
   * set keeps track of projects that were newly tagged (they will have to
   * be communicated to other nodes). */
  bool accept(Row &row) override {
    int pid = row.get_int(0);
    int uid = row.get_int(1);
    if (uSet.test(uid))
      if (!pSet.test(pid)) {
        pSet.set(pid);
        newProjects.set(pid);
      }
    return false;
  }
};

generate_classmap(IDSet, IntToBool, IntArray, BoolArray, int, bool);

/** Union to put all of a into b **/
void combine_into_right(IDSet &a, IDSet &b) {
  IntArray *a_ids = a.keys();
  for (size_t i = 0; i < a_ids->size(); i++)
    b.put(a_ids->get(i), true);
  delete a_ids;
}

class IDSetWriter : public Writer {
public:
  IntArray *ids_;
  size_t idx_ = 0;

  IDSetWriter(IDSet &set) { ids_ = set.keys(); }
  ~IDSetWriter() { delete ids_; }

  bool done() { return idx_ >= ids_->size(); }
  bool accept(Row &row) {
    row.set(0, ids_->get(idx_++));
    return true;
  }
};

class IDSetUpdater : public Rower {
public:
  IDSet &set_;
  IDSetUpdater(IDSet &set) : set_(set) {}

  /** Assume a row with at least one column of type I. Assumes that there
   * are no missing. Reads the value and sets the corresponding position.
   * The return value is irrelevant here. */
  bool accept(Row &row) {
    set_.put(row.get_int(0), true);
    return true;
  }
};

class IDProjectsTagger : public Rower {
public:
  IDSet &uSet_;      // set of collaborator
  IDSet &pSet_;      // set of projects of collaborators
  IDSet newProjects; // newly tagged collaborator projects

  IDProjectsTagger(IDSet &uSet, IDSet &pSet) : uSet_(uSet), pSet_(pSet) {}

  /** The data frame must have at least two integer columns. The newProject
   * set keeps track of projects that were newly tagged (they will have to
   * be communicated to other nodes). */
  bool accept(Row &row) {
    int pid = row.get_int(0);
    int uid = row.get_int(1);
    if (uSet_.contains_key(uid))
      if (!pSet_.contains_key(pid)) {
        pSet_.put(pid, true);
        newProjects.put(pid, true);
      }
    return true;
  }
};

class IDUsersTagger : public Rower {
public:
  IDSet &pSet_;   // set of projects of collaborators
  IDSet &uSet_;   // set of collaborator
  IDSet newUsers; // newly tagged collaborator projects

  IDUsersTagger(IDSet &pSet, IDSet &uSet) : pSet_(pSet), uSet_(uSet) {}

  /** The data frame must have at least two integer columns. The newProject
   * set keeps track of projects that were newly tagged (they will have to
   * be communicated to other nodes). */
  bool accept(Row &row) {
    int pid = row.get_int(0);
    int uid = row.get_int(1);
    if (pSet_.contains_key(pid))
      if (!uSet_.contains_key(uid)) {
        uSet_.put(uid, true);
        newUsers.put(uid, true);
      }
    return true;
  }
};

/***************************************************************************
 * The UserTagger is a reader that is mapped over commits, and marks all of
 * the users which commmitted to a project to which a collaborator of Linus
 * also committed as an author. The commit dataframe has the form:
 *    pid x uid x uid
 * where the pid is the idefntifier of a project and the uids are the
 * identifiers of the author and committer.
 *************************************************************************/
class UsersTagger : public Rower {
public:
  Set &pSet;
  Set &uSet;
  Set newUsers;

  UsersTagger(Set &pSet, Set &uSet, DataFrame *users)
      : pSet(pSet), uSet(uSet), newUsers(users->nrows()) {}

  bool accept(Row &row) override {
    int pid = row.get_int(0);
    int uid = row.get_int(1);
    if (pSet.test(pid))
      if (!uSet.test(uid)) {
        uSet.set(uid);
        newUsers.set(uid);
      }
    return false;
  }
};

/*************************************************************************
 * This computes the collaborators of Linus Torvalds.
 * is the linus example using the adapter.  And slightly revised
 *   algorithm that only ever trades the deltas.
 **************************************************************************/
class Linus : public Application {
public:
  size_t DEGREES = 4; // How many degrees of separation form linus?
  int LINUS = 4967;   // The uid of Linus (offset in the user df)
  const char *PROJ = "data/projects-tiny.ltgt";
  const char *USER = "data/users-tiny.ltgt";
  const char *COMM = "data/commits-tiny.ltgt";
  DataFrame *projects; //  pid x project name
  DataFrame *users;    // uid x user name
  DataFrame *commits;  // pid x uid x uid
  IDSet uSet;          // Linus' collaborators
  IDSet pSet;          // projects of collaborators

  Linus(size_t idx, Network *net) : Application(idx, net) {}

  /** Compute DEGREES of Linus. */
  void run_() override {
    readInput();
    for (size_t i = 0; i < DEGREES; i++)
      step(i);

    if (this_node() == 0) {
      stop_all();
    }
  }

  /** Node 0 reads three files, cointainng projects, users and commits, and
   *  creates thre dataframes. All other nodes wait and load the three
   *  dataframes. Once we know the size of users and projects, we create
   *  sets of each (uSet and pSet). We also output a data frame with a the
   *  'tagged' users. At this point the dataframe consists of only
   *  Linus. **/
  void readInput() {
    Key pK("projs");
    Key uK("usrs");
    Key cK("comts");
    if (this_node() == 0) {
      pln("Reading...");
      projects = DataFrame::fromFile(PROJ, &pK, this_store());
      p("    ").p(projects->nrows()).pln(" projects");
      users = DataFrame::fromFile(USER, &uK, this_store());
      p("    ").p(users->nrows()).pln(" users");
      commits = DataFrame::fromFile(COMM, &cK, this_store());
      p("    ").p(commits->nrows()).pln(" commits");
      // This dataframe contains the id of Linus.
      delete DataFrame::fromScalarI(new Key("users-0-0"), this_store(), LINUS);
    } else {
      projects = this_store()->get_and_wait(&pK);
      users = this_store()->get_and_wait(&uK);
      commits = this_store()->get_and_wait(&cK);
    }
  }

  /** Performs a step of the linus calculation. It operates over the three
   *  datafrrames (projects, users, commits), the sets of tagged users and
   *  projects, and the users added in the previous round. */
  void step(int stage) {
    p("Stage ").pln(stage);
    // Key of the shape: users-stage-0
    Key uK(StrBuff("users-").c(stage).c("-0").get());
    // A df with all the users added on the previous round
    DataFrame *newUsers = this_store()->get_and_wait(&uK);
    IDSet delta;
    IDSetUpdater upd(delta);
    newUsers->distributed_map(upd); // all of the new users are copied to delta.
    delete newUsers;
    IDProjectsTagger ptagger(delta, pSet);
    commits->local_map(ptagger); // marking all projects touched by delta
    merge(ptagger.newProjects, "projects-", stage);
    // pSet->union_(ptagger.newProjects);
    combine_into_right(ptagger.newProjects, pSet);
    IDUsersTagger utagger(ptagger.newProjects, uSet);
    commits->local_map(utagger);
    merge(utagger.newUsers, "users-", stage + 1);
    // uSet->union_(utagger.newUsers);
    combine_into_right(utagger.newUsers, uSet);
    p("    after stage ").p(stage).pln(":");
    p("        tagged projects: ").pln(pSet.size());
    p("        tagged users: ").pln(uSet.size());
  }

  /** Gather updates to the given set from all the nodes in the systems.
   * The union of those updates is then published as dataframe.  The key
   * used for the otuput is of the form "name-stage-0" where name is either
   * 'users' or 'projects', stage is the degree of separation being
   * computed.
   */
  void merge(IDSet &set, char const *name, int stage) {
    if (this_node() == 0) {
      for (size_t i = 1; i < arg.num_nodes; ++i) {
        Key nK(StrBuff(name).c(stage).c("-").c(i).get(), i);
        DataFrame *delta = this_store()->get_and_wait(&nK);
        p("    received delta of ")
            .p(delta->nrows())
            .p(" elements from node ")
            .pln(i);
        IDSetUpdater upd(set);
        delta->distributed_map(upd);
        delete delta;
      }
      p("    storing ").p(set.size()).pln(" merged elements");
      IDSetWriter writer(set);
      Key k(StrBuff(name).c(stage).c("-0").get(), this_node());
      delete DataFrame::fromVisitor(&k, this_store(), "I", writer);
    } else {
      p("    sending ").p(set.size()).pln(" elements to master node");
      IDSetWriter writer(set);
      Key k(StrBuff(name).c(stage).c("-").c(this_node()).get(), this_node());
      delete DataFrame::fromVisitor(&k, this_store(), "I", writer);
      Key mK(StrBuff(name).c(stage).c("-0").get());
      DataFrame *merged = this_store()->get_and_wait(&mK);
      p("    receiving ").p(merged->nrows()).pln(" merged elements");
      IDSetUpdater upd(set);
      merged->distributed_map(upd);
      delete merged;
    }
  }
};