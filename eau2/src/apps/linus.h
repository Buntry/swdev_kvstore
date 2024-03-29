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
// generate_classmap(Set, IntToBool, IntArray, BoolArray, int, bool);
class Set : public Object {
public:
  bool *vals_ = nullptr;
  size_t sz_ = 0;    // total capacity
  size_t elems_ = 0; // number of elements

  /** Creates a set of the same size as the dataframe. **/
  Set(DataFrame *df) : Set(df->nrows()) {}

  /** Creates a set of the given size **/
  Set(size_t sz) : vals_(new bool[sz]), sz_(sz) {
    for (size_t i = 0; i < sz_; i++) {
      vals_[i] = false;
    }
  }

  ~Set() { delete[] vals_; }

  /** Add idx to the set. If an idx is out of bounds, ignore it. **/
  void set(size_t idx) {
    if (idx >= sz_ || vals_[idx])
      return;
    vals_[idx] = true;
    elems_++;
  }

  /** Is idx in the set?  See comment for set(). */
  bool test(size_t idx) {
    if (idx >= sz_)
      return false; // ignoring out of bound reads
    return vals_[idx];
  }

  size_t size() { return elems_; }
  size_t capacity() { return sz_; }

  /** Performs set union in place. */
  void union_(Set &from) {
    for (size_t i = 0; i < from.sz_; i++)
      if (from.test(i))
        set(i);
  }
};

/*****************************************************************************
 * A SetWriter copies all the values present in the set into a one-column
 * dataframe. The data contains all the values in the set. The dataframe has
 * at least one integer column.
 ****************************************************************************/
class SetWriter : public Writer {
public:
  Set &set_;
  size_t idx_ = 0;
  size_t num_accepted_ = 0;

  SetWriter(Set &set) : set_(set) {}

  /** Skip over false values and stop when the entire set has been seen. **/
  bool done() {
    while (idx_ < set_.capacity() && !set_.test(idx_)) {
      idx_++;
    }
    return idx_ >= set_.capacity();
  }

  bool accept(Row &row) {
    row.set(0, (int)idx_++);
    return true;
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
  Set *uSet;           // Linus' collaborators
  Set *pSet;           // projects of collaborators

  Linus(size_t idx, Network *net) : Application(idx, net) {}
  ~Linus() {
    delete projects;
    delete users;
    delete commits;
    delete uSet;
    delete pSet;
  }

  /** Compute DEGREES of Linus. */
  void run_() override {
    readInput();
    for (size_t i = 0; i < DEGREES; i++)
      step(i);

    if (this_node() == last_node()) {
      sleep(100);
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
      Key linus("users-0-0");
      delete DataFrame::fromScalarI(&linus, this_store(), LINUS);
    } else {
      projects = this_store()->get_and_wait(&pK);
      users = this_store()->get_and_wait(&uK);
      commits = this_store()->get_and_wait(&cK);
    }
    uSet = new Set(users);
    pSet = new Set(projects);
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
    Set delta(users);
    SetUpdater upd(delta);
    newUsers->distributed_map(upd); // all of the new users are copied to delta.
    delete newUsers;

    ProjectsTagger ptagger(delta, *pSet, projects);
    commits->local_map(ptagger); // marking all projects touched by delta
    merge(ptagger.newProjects, "projects-", stage);
    pSet->union_(ptagger.newProjects);
    UsersTagger utagger(ptagger.newProjects, *uSet, users);
    commits->local_map(utagger);
    merge(utagger.newUsers, "users-", stage + 1);
    uSet->union_(utagger.newUsers);
    p("    after stage ").p(stage).pln(":");
    p("        tagged projects: ").pln(pSet->size());
    p("        tagged users: ").pln(uSet->size());
  }

  /** Gather updates to the given set from all the nodes in the systems.
   * The union of those updates is then published as dataframe.  The key
   * used for the otuput is of the form "name-stage-0" where name is either
   * 'users' or 'projects', stage is the degree of separation being
   * computed.
   */
  void merge(Set &set, char const *name, int stage) {
    if (this_node() == 0) {
      for (size_t i = 1; i < arg.num_nodes; i++) {
        Key nK(StrBuff(name).c(stage).c("-").c(i).get(), i);
        DataFrame *delta = this_store()->get_and_wait(&nK);
        p("    received delta of ")
            .p(delta->nrows())
            .p(" elements from node ")
            .pln(i);
        SetUpdater upd(set);
        delta->distributed_map(upd);
        delete delta;
      }
      p("    storing ").p(set.size()).pln(" merged elements");
      SetWriter writer(set);
      Key k(StrBuff(name).c(stage).c("-0").get(), this_node());
      delete DataFrame::fromVisitor(&k, this_store(), "I", writer);
    } else {
      p("    sending ").p(set.size()).pln(" elements to master node");
      SetWriter writer(set);
      Key k(StrBuff(name).c(stage).c("-").c(this_node()).get(), this_node());
      delete DataFrame::fromVisitor(&k, this_store(), "I", writer);
      Key mK(StrBuff(name).c(stage).c("-0").get());
      DataFrame *merged = this_store()->get_and_wait(&mK);
      p("    receiving ").p(merged->nrows()).pln(" merged elements");
      SetUpdater upd(set);
      merged->distributed_map(upd);
      delete merged;
    }
  }
};