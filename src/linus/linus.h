#include "../dataframe/modified_dataframe.h"
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


 class Args : public Object {
 public:
   size_t num_nodes = 0;
   size_t nodeIndex = 0;

   Args() {}

   ~Args() {}

   void parse(int argc, char ** argv) {
     int index = 1; // skipping the first argument
     while (index < argc) {
         if (strcmp(argv[index], "-nodeIndex") == 0) {
             nodeIndex = atoi(argv[index + 1]);
             index+=2;
         }
         else if (strcmp(argv[index], "-nodes") == 0) {
             num_nodes = atoi(argv[index + 1]);
             index+=2;
         }
         else {
             affirm(false, "Invalid input argument");
         }
     }
     std::cout << "Found -nodeIndex " << nodeIndex << " and -nodes " << num_nodes << "\n";
   }
 };


/**************************************************************************
 * A bit set contains size() booleans that are initialize to false and can
 * be set to true with the set() method. The test() method returns the
 * value. Does not grow.
 ************************************************************************/
class Set {
public:
  bool* vals_;  // owned; data
  size_t size_; // number of elements

  /** Creates a set of the same size as the dataframe. */
  Set(DataFrame* df) : Set(df->nrows()) {}

  /** Creates a set of the given size. */
  Set(size_t sz) :  vals_(new bool[sz]), size_(sz) {
      for(size_t i = 0; i < size_; i++)
          vals_[i] = false;
  }

  ~Set() { delete[] vals_; }

  /** Add idx to the set. If idx is out of bound, ignore it.  Out of bound
   *  values can occur if there are references to pids or uids in commits
   *  that did not appear in projects or users.
   */
  void set(size_t idx) {
    if (idx >= size_ ) return; // ignoring out of bound writes
    vals_[idx] = true;
  }

  /** Is idx in the set?  See comment for set(). */
  bool test(size_t idx) {
    if (idx >= size_) return true; // ignoring out of bound reads
    return vals_[idx];
  }

  size_t size() { return size_; }

  /** Performs set union in place. */
  void union_(Set& from) {
    for (size_t i = 0; i < from.size_; i++)
      if (from.test(i))
	set(i);
  }
};


/*******************************************************************************
 * A SetUpdater is a reader that gets the first column of the data frame and
 * sets the corresponding value in the given set.
 ******************************************************************************/
class SetUpdater : public Reader {
public:
  Set& set_; // set to update

  SetUpdater(Set& set): set_(set) {}

  /** Assume a row with at least one column of type I. Assumes that there
   * are no missing. Reads the value and sets the corresponding position.
   * The return value is irrelevant here. */
  bool visit(Row & row) { set_.set(row.get_int(0));  return false; }

};

/*****************************************************************************
 * A SetWriter copies all the values present in the set into a one-column
 * dataframe. The data contains all the values in the set. The dataframe has
 * at least one integer column.
 ****************************************************************************/
class SetWriter: public Writer {
public:
  Set& set_; // set to read from
  int i_ = 0;  // position in set

  SetWriter(Set& set): set_(set) { }

  /** Skip over false values and stop when the entire set has been seen */
  bool done() {
    while (i_ < set_.size_ && set_.test(i_) == false) ++i_;
    return i_ == set_.size_;
  }

  bool visit(Row & row) { row.set(0, i_++); return 0;}
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
class ProjectsTagger : public Reader {
public:
  Set& uSet; // set of collaborator
  Set& pSet; // set of projects of collaborators
  Set newProjects;  // newly tagged collaborator projects

  ProjectsTagger(Set& uSet, Set& pSet, DataFrame* proj):
    uSet(uSet), pSet(pSet), newProjects(proj) {}

  /** The data frame must have at least two integer columns. The newProject
   * set keeps track of projects that were newly tagged (they will have to
   * be communicated to other nodes). */
  bool visit(Row & row) override {
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
class UsersTagger : public Reader {
public:
  Set& pSet;
  Set& uSet;
  Set newUsers;

  UsersTagger(Set& pSet,Set& uSet, DataFrame* users):
    pSet(pSet), uSet(uSet), newUsers(users->nrows()) { }

  bool visit(Row & row) override {
    int pid = row.get_int(0);
    int uid = row.get_int(1);
    if (pSet.test(pid))
      if(!uSet.test(uid)) {
	uSet.set(uid);
	newUsers.set(uid);
      }
    return false;
  }
};

class Application : public Object {
 public:
   size_t nodeIndex;
   size_t num_nodes;
   KVStore *kv;

   Application(size_t nodeIndex_, size_t num_nodes_) {
     nodeIndex = nodeIndex_;
     num_nodes = num_nodes_;
     kv = new KVStore(nodeIndex, num_nodes);
   }

   // Application() {
   //   nodeIndex = 0;
   //   num_nodes = 0;
   //   kv = nullptr;
   // }

   ~Application() {

   }

   virtual void run_() {};


};

/*************************************************************************
 * This computes the collaborators of Linus Torvalds.
 * is the linus example using the adapter.  And slightly revised
 *   algorithm that only ever trades the deltas.
 **************************************************************************/
class Linus : public Application {
public:
  int DEGREES = 4;  // How many degrees of separation form linus?
  int LINUS = 4967;   // The uid of Linus (offset in the user df)
  //const char* PROJ = "datasets/projects.ltgt";
  //const char* USER = "datasets/users.ltgt";
  //const char* COMM = "datasets/commits.ltgt";
  const char* PROJ = "../src/linus/datasets/original/projects.sor";
  const char* USER = "../src/linus/datasets/original/users.sor";
  const char* COMM = "../src/linus/datasets/original/commits.sor";
  // const char* PROJ = "../src/linus/datasets/projects.sor";
  // const char* USER = "../src/linus/datasets/users.sor";
  // const char* COMM = "../src/linus/datasets/commits.sor";
  DataFrame* projects; //  pid x project name
  DataFrame* users;  // uid x user name
  DataFrame* commits;  // pid x uid x uid
  Set* uSet; // Linus' collaborators
  Set* pSet; // projects of collaborators
  Args* arg;

  Linus(Args* arg_): Application(arg_->nodeIndex, arg_->num_nodes) {
    arg = arg_;
  }

  /** Compute DEGREES of Linus.  */
  void run_() override {
    readInput();
    std::cout << "Starting Step....................\n";
    sleep(2);
    for (size_t i = 0; i < DEGREES; i++) step(i);
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
    Schema *s = new Schema();
    DataFrame *df = new DataFrame(*s);
    if (nodeIndex == 0) {
      std::cout << "Reading...\n";
      projects = df->fromFile(PROJ, pK.clone(), kv);
      std::cout << "    "<< projects->nrows() << "\n  projects";
      // projects->print();
      // sleep(10);
      users = df->fromFile(USER, uK.clone(), kv);
      std::cout << "    "<< users->nrows() <<  users << "\n";
      commits = df->fromFile(COMM, cK.clone(), kv);
      std::cout << "    "<< projects->nrows() << "\n commits";
      // This dataframe contains the id of Linus.
      df->fromScalar(new Key("users-0-0"), kv, LINUS);
    } else {
      std::cout << "Starting df...\n";
       projects = dynamic_cast<DataFrame*>(kv->get(&pK));
       projects->print();
       sleep(10);
       std::cout << "Finish df 1...\n";
       users = dynamic_cast<DataFrame*>(kv->get(&uK));
       std::cout << "Finish df 2...\n";
       commits = dynamic_cast<DataFrame*>(kv->get(&cK));
       std::cout << "Finish df 3...\n";
    }
    std::cout << "Starting sets...\n";
    uSet = new Set(users);
    pSet = new Set(projects);
    std::cout << "Done wiht sets...\n";
    delete df;
    delete s;
 }

 /** Performs a step of the linus calculation. It operates over the three
  *  datafrrames (projects, users, commits), the sets of tagged users and
  *  projects, and the users added in the previous round. */
  void step(int stage) {
    // kv->store->store->printall();
    // std::cout << "inside STEP \n";
    // std::cout << "Stage \n" << stage << "\n";
    // Key of the shape: users-stage-0
    Key uK(StrBuff("users-").c(stage).c("-0").get()->c_str());
    // A df with all the users added on the previous round
    DataFrame* newUsers = dynamic_cast<DataFrame*>(kv->waitAndGet(&uK));
    Set delta(users);
    SetUpdater upd(delta);
    newUsers->map(upd); // all of the new users are copied to delta.
    delete newUsers;
    ProjectsTagger ptagger(delta, *pSet, projects);
    commits->map(ptagger); // marking all projects touched by delta
    merge(ptagger.newProjects, "projects-", stage);
    pSet->union_(ptagger.newProjects); //
    UsersTagger utagger(ptagger.newProjects, *uSet, users);
    commits->map(utagger);
    merge(utagger.newUsers, "users-", stage + 1);
    uSet->union_(utagger.newUsers);
    std::cout << "    after stage "<< stage << ":\n";
    std::cout << "        tagged projects: "<< pSet->size() << "\n";
    std::cout << "        tagged users: "<< uSet->size() << "\n";
  }

  /** Gather updates to the given set from all the nodes in the systems.
   * The union of those updates is then published as dataframe.  The key
   * used for the otuput is of the form "name-stage-0" where name is either
   * 'users' or 'projects', stage is the degree of separation being
   * computed.
   */
  void merge(Set& set, char const* name, int stage) {
    Schema *s = new Schema();
    DataFrame *df = new DataFrame(*s);
    if (nodeIndex == 0) {
      for (size_t i = 1; i < arg->num_nodes; ++i) {
      	Key nK(StrBuff(name).c(stage).c("-").c(i).get()->c_str());
      	DataFrame* delta = dynamic_cast<DataFrame*>(kv->waitAndGet(&nK));
        std::cout << "    received delta of " << delta->nrows() << " elements from node " << i << "\n";
      	SetUpdater upd(set);
      	delta->map(upd);
      	delete delta;
      }
      std::cout << "    storing "<< set.size() << " merged elements\n";
      SetWriter writer(set);
      Key k(StrBuff(name).c(stage).c("-0").get()->c_str());
      String *colType = new String("I");
      df->fromVisitor(&k, kv, colType->c_str(), &writer);
      delete colType;
    } else {
      std::cout << "    storing "<< set.size() << " elements to master node\n";
      SetWriter writer(set);
      Key k(StrBuff(name).c(stage).c("-").c(nodeIndex).get()->c_str());
      String *colType = new String("I");
      df->fromVisitor(&k, kv, colType->c_str(), &writer);
      delete colType;
      Key mK(StrBuff(name).c(stage).c("-0").get()->c_str());
      DataFrame* merged = dynamic_cast<DataFrame*>(kv->waitAndGet(&mK));
      std::cout << "    receiving "<< merged->nrows() << " merged elements\n";
      SetUpdater upd(set);
      merged->map(upd);
      delete merged;
    }
    delete s;
    delete df;
  }
}; // Linus
