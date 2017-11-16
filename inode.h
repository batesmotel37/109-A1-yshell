// $Id: inode.h,v 1.15 2015-07-12 04:51:51-07 - - $

#ifndef __INODE_H__
#define __INODE_H__

#include <exception>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
using namespace std;

#include "util.h"

//
// inode_t -
//    An inode is either a directory or a plain file.
//

enum inode_t {PLAIN_INODE, DIR_INODE};
class inode;
class file_base;
class plain_file;
class directory;
using inode_ptr = shared_ptr<inode>;
using file_base_ptr = shared_ptr<file_base>;
using plain_file_ptr = shared_ptr<plain_file>;
using directory_ptr = shared_ptr<directory>;

//
// inode_state -
//    A small convenient class to maintain the state of the simulated
//    process:  the root (/), the current directory (.), and the
//    prompt.
//

class inode_state {
   friend class inode;
   friend ostream& operator<< (ostream& out, const inode_state&);
   private:
      inode_state (const inode_state&) = delete; // copy ctor
      inode_state& operator= (const inode_state&) = delete; // op=
      inode_ptr root {nullptr};
      inode_ptr cwd {nullptr};
      string prompt {"% "};
   public:
      inode_state();
      inode_ptr findPath(string dir);
      void setPrompt(const string& newPrompt);
      string getPrompt();
      string cwd_name();
      void printDir(string dirName);
      void printDir_recur(string dirName);
      void setCwd(string dir);
//    inode_ptr changeDir(string dir);
      //inode_ptr getCwd();
      //inode_ptr getRoot();
      void mkdir(const string& dirname);
      void insert(string dirname,inode_ptr parent);
      void mkfile(const string& filename,const wordvec& words);
      const wordvec& cat(const string& filename);
      //inode& mkdir(const string& dirname);
      void remove(const string& filename);
      void remove_recur(const string& filename);
      void clean();
};


//
// class inode -
//
// inode ctor -
//    Create a new inode of the given type.
// get_inode_nr -
//    Retrieves the serial number of the inode.  Inode numbers are
//    allocated in sequence by small integer.
// size -
//    Returns the size of an inode.  For a directory, this is the
//    number of dirents.  For a text file, the number of characters
//    when printed (the sum of the lengths of each word, plus the
//    number of words.
//    

class inode {
   friend class inode_state;
   private:
      static int next_inode_nr;
      int inode_nr;
      inode_t type;
      file_base_ptr contents;
   public:
      inode (inode_t init_type);//,inode_ptr parent);
      int get_inode_nr() const;
      inode_t getType();
      int getSize();
      string parent_name();
      string name(int child_no);
      void printDir();
      void printDir_recur();
      inode_ptr findPath(string dirName);
      void mkdir(const string& dirname,inode_ptr parent);
      void insert(string dirname,inode_ptr parent);
      void mkfile (const string& filename, const wordvec& newdata);
      void writefile(const wordvec& newdata);
      const wordvec& read(const string& filename);
      const wordvec& readfile();
      //inode& mkdir(const string& dirname);//,inode_ptr parent);
      void remove(const string& filename);
      void remove_recur(const string& filename);
      void clean();
};

//
// class file_base -
//
// Just a base class at which an inode can point.  No data or
// functions.  Makes the synthesized members useable only from
// the derived classes.
//

class file_base {
   protected:
      file_base () = default;
      file_base (const file_base&) = default;
      file_base (file_base&&) = default;
      file_base& operator= (const file_base&) = default;
      file_base& operator= (file_base&&) = default;
      virtual ~file_base () = default;
      virtual size_t size() const = 0;
   public:
      int getSize();
      friend plain_file_ptr plain_file_ptr_of (file_base_ptr);
      friend directory_ptr directory_ptr_of (file_base_ptr);
};


//
// class plain_file -
//
// Used to hold data.
// synthesized default ctor -
//    Default vector<string> is a an empty vector.
// readfile -
//    Returns a copy of the contents of the wordvec in the file.
//    Throws an yshell_exn for a directory.
// writefile -
//    Replaces the contents of a file with new contents.
//    Throws an yshell_exn for a directory.
//

class plain_file: public file_base {
   private:
      wordvec data;
   public:
      size_t size() const override;
      const wordvec& readfile() const;
      void writefile (const wordvec& newdata);
};

//
// class directory -
//
// Used to map filenames onto inode pointers.
// default ctor -
//    Creates a new map with keys "." and "..".
// remove -
//    Removes the file or subdirectory from the current inode.
//    Throws an yshell_exn if this is not a directory, the file
//    does not exist, or the subdirectory is not empty.
//    Here empty means the only entries are dot (.) and dotdot (..).
// mkdir -
//    Creates a new directory under the current directory and 
//    immediately adds the directories dot (.) and dotdot (..) to it.
//    Note that the parent (..) of / is / itself.  It is an error
//    if the entry already exists.
// mkfile -
//    Create a new empty text file with the given name.  Error if
//    a dirent with that name exists.

class directory: public file_base {
   private:
      map<string,inode_ptr> dirents;
   public:
      //directory(inode_ptr self);//, inode_ptr parent);
      size_t size() const override;
      void remove (const string& filename);
      void remove_recur(const string& filename);
      void mkdir (const string& dirname,inode_ptr parent);
      void insert(string dirname, inode_ptr parent);
      //void mkdir(const string& dirname, inode_ptr self);
      //inode& mkdir (const string& dirname);//, inode_ptr parent);
      void mkfile(const string& filename, const wordvec& newdata);
      //inode& mkfile (const string& filename,const wordvec& newdata);
      const wordvec& read(const string& filename);
      string parent(int child_no);
      string name(int child_no);
      inode_ptr findPath(string dirName);
      //pair<string,inode_ptr> print_dirents();
      void print_dirents_recur();
      void print_dirents();
      void clean();
};

#endif

