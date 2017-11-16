
#include <iostream>
#include <stdexcept>

using namespace std;

#include "debug.h"
#include "inode.h"

int inode::next_inode_nr {1};
/*
   directory::directory(inode_ptr self){//, inode_ptr parent){
   auto dot=pair<string,inode_ptr>(".",self);
   //auto dotdot=pair<string,inode_ptr>("..",parent);
   dirents.insert(dot);
   //dirents.insert(dotdot);
}
*/
inode::inode(inode_t init_type)://,inode_ptr parent):
   inode_nr (next_inode_nr++), type (init_type)
{
   switch (type) {
      case PLAIN_INODE:
           contents = make_shared<plain_file>();
           break;
      case DIR_INODE:
           contents = make_shared<directory>();//this);//,parent);
           //auto dir=directory_ptr_of(contents);
           //dir->mkdir("..",parent);
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

/*
 * constructor for inode_state
 *
 * inode_state points at two indoes, the root directory
 * which is assigned to point to a DIR_INODE here
 * and the current working directory (cwd) which
 * is pointed at root initially
 */
inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt << "\"");
   root=make_shared<inode>(inode(DIR_INODE));//,root));
   cwd=root;
   root->insert("..",root);
   root->insert(".",root);
}


int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

inode_t inode:: getType(){
   return type;
}

plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) {
   plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
   if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
   return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) {
   directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
   if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
   return dirptr;
}

size_t plain_file::size() const {
   size_t size {0};
   if(data.size()!=0){
      for(auto itor=data.begin();itor!=data.end();++itor){
         size+=(*itor).length()+1;
      }
      size--;
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data=words;
//cout<<words.at(0)<<endl;
}

/*
 * the size of a directory is the number of entries in its map
 */
size_t directory::size() const {
   size_t size {0};
   size=dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

/*
 */
void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   if(dirents.find(filename)!=dirents.end()){
      //cout<<"found it"<<endl;
      inode_ptr target=dirents.at(filename);
      if(target->getSize()==2||target->getType()==PLAIN_INODE){//2
         //const auto entry=dirents.find(filename);
         //entry.second=nullptr;
         target->clean();
         //dirents.at(filename)=nullptr;
         dirents.erase(filename);//cout<<"It's empty too!"<<endl;
      }
      else
         throw yshell_exn("Directory "+filename+
                          " is not empty and cannot be removed\n");
         //cerr<<"directory \""<<filename
         //  <<"\" is not empty and therefore cannot be removed"<<endl;
   }
   else
      throw yshell_exn("No file or directory \""+filename+
                       "\" found\n");
      //cerr<<"No file or directory \""<<filename<<"\" found"<<endl;
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root<<", cwd = "<<state.cwd;
   return out;
}

/*---------------------------------------------------------------------
 *
 * findPath()
 *
 * is passed a string which can be parsed by '/' to show a series of
 * directories which can be traversed sequentially to arrive at a
 * destination. This destination is then returned as an inode_ptr
 *
 * Is called indirectly by fn_cd, fn_make, fn_makdir, fn_ls, fn_rm,
 * fn_rmr, and fn_lsr
 */
inode_ptr directory::findPath(string dirName){
   if(dirents.find(dirName)!=dirents.end()){
      auto itor=dirents.find(dirName);
//   cout<<itor->first<<" "<<itor->second<<endl;
      inode_ptr dest=itor->second;
      inode_t type=dest->getType();
      if(type==DIR_INODE)
         return itor->second;
      else
         throw yshell_exn(dirName+" is a file\n");
         //cerr<<dirName<<" is a file"<<endl;
   }
   else{
      throw yshell_exn("The directory \""+dirName+
                       "\" does not exits\n");
      //cerr<<"The directory \""<<dirName<<"\" does not exist"<<endl;
      //return dirents.at("..");
   }
}

inode_ptr inode::findPath(string dirName){
   auto dir=directory_ptr_of(contents);
   return dir->findPath(dirName);
}

inode_ptr inode_state::findPath(string dir){
   inode_ptr i;
   try{
      if(dir.front()=='/')
         i=root;
      else
         i=cwd;
      wordvec path=split(dir,"/");
      for(auto itor=path.begin();itor!=path.end();++itor){
         //cout<<*itor<<endl;
         i=i->findPath(*itor);
         //cout<<i<<endl;
      }
      return i;
   }
   catch(exception& e){
      throw yshell_exn("Error: "+dir+" is not a directory!\n");
      //cerr<<"Error: "<<dir<<": is not a directory!"<<endl;
      return cwd;
   }
   return cwd;
}

//---------------------------------------------------------------------


/*---------------------------------------------------------------------
 *
 * cat()
 *
 */
const wordvec& inode::readfile(){
   if(type==PLAIN_INODE){
      auto file=plain_file_ptr_of(contents);
      return file->readfile();
   }
   else
      throw yshell_exn("Error: trying to read from a directory\n");
}

const wordvec& directory::read(const string& filename){
   if(dirents.find(filename)!=dirents.end()){
      return dirents.at(filename)->readfile();
   }
   else
      throw yshell_exn("Error: "+filename+" does not exist in this"+ 
                       +"directory\n");
//   const wordvec& w {};
//   return w;
}

const wordvec& inode::read(const string& filename){
   auto dir=directory_ptr_of(contents);
   return dir->read(filename);
//auto file=plain_file_ptr_of(contents);
//return file->readfile();
}

const wordvec& inode_state::cat(const string& filename){
   if(filename.front()=='/'){
      inode_ptr i=root;
      wordvec destination=split(filename,"/");
      if(destination.size()>1){
         string dest=destination.back();
         string path="";
         for(auto itor=destination.begin();itor+1!=destination.end()
                                          ;++itor){
            path+=*itor+"/";
         }
         i=findPath(path);
         return i->read(dest);
      }
      else
         return root->read(filename);
   }
   else{
      wordvec destination=split(filename,"/");
      if(destination.size()>1){
         string path="";
         for(auto itor=destination.begin();itor+1!=destination.end()
                                          ;++itor){
            path+=*itor+"/";
         }
         inode_ptr i=findPath(path);
         string dest=destination.back();
         return i->read(dest);
      }
      else{
         return cwd->read(filename);
      }
   }
}
//---------------------------------------------------------------------

/*
 * Called on by fn_cd
 *
 * changes the value of cwd, either to root if given an empty string
 * or to a given directory otherwise
 * this other directory is found using findPath()
 */
void inode_state::setCwd(string dir){
   if(dir.length()==0)
      cwd=root;
   else{
      inode_ptr newDir=findPath(dir);
      cwd=newDir;
   }
}

/*--------------------------------------------------------------------
 *
 * directory name()
 *
 * returns the name of the current directory
 *
 * if the current directory is root, the name is "/"
 * otherwise the name of the directory name can be found by searching
 * the map of the parent inode (hence the first conditional, root only
 * has itself as a parent so "/" is not an actual name stored in any
 * directory)
 * the searching of the map is done by checking the inode_ptr feild and
 * comparing its inode number to cwd, if they're equal, we return the
 * string feild
 *
 * if nothing is found in the map and error is printed and an
 * exception is thrown
 */
string directory::name(int child_no){
   for(auto dir:dirents){
//cout<<dir.first;
//cout<<" ";
//cout<<dir.second->get_inode_nr()<<"=?"<<child_no<<endl;
      if(dir.second->get_inode_nr()==child_no){
         if(dir.second->get_inode_nr()==1&&child_no==1)
            return "/";
         return dir.first;
      }
   }
   throw yshell_exn("Error: no name for this directory found\n");
   //cerr<<"Error: no name for this directory found"<<endl;
   return "";
}

string inode::name(int child_no){
   auto dir=directory_ptr_of(contents);
   return dir->name(child_no);
}

string directory::parent(int child_no){//return "cwd";
   for(auto dir:dirents){
      if(dir.first.compare("..")==0){
         return dir.second->name(child_no);
         //if(dir.second->get_inode_nr()==child_no)
         //return dir.first;
      }
   }
   throw yshell_exn("Error: no name for this directory found\n");
   //cerr<<"Error: no name for this directory found"<<endl;
   return "";
}

string inode::parent_name(){//return "cwd";
/*auto dir=directory_ptr_of(contents);
     dir->print_dirents();*/
   auto dir=directory_ptr_of(contents);
   return dir->parent(get_inode_nr());
}

string inode_state::cwd_name(){
   //check if we are at the root node, b/c we cannot find its name
   //through parentage
      //if(cwd==root)
      //return "/";
   /*
 * if the node is not root we need to find the parent
 * which requires that we access the dirents map in dictionary
 * to do this we call inode.parent_name(),
 * which will call dictionary.parent()
 */
   //else//code rest of this in step 8
      return cwd->parent_name();
}
//--------------------------------------------------------------------

/*--------------------------------------------------------------------
 *
 * printDirectory()
 *
 * called by fn_ls(). If no targets are passed in, then the contents of
 * the current directory are all that's printed out
 * otherwise a specific directory is found using followPath()
 * and its contents are printed.
 */
void directory::print_dirents(){
   if(!dirents.empty()){
      //iterate over each entry in the map
      //print out its inode number, its size, and its name
      for(const auto& entry:dirents){
         string name=entry.first;
         inode_ptr file=entry.second;
         cout<<"\t"<<file->get_inode_nr()<<"\t"<<file->getSize()
             <<"\t"<<name<<"\t"<<endl;
      }
   }
   else
      throw yshell_exn("This directory is empty\n");
      //cerr<<"This directory is empty"<<endl;
}

int file_base::getSize(){
   return size();
}

int inode::getSize(){
   return contents->getSize();
}

void inode::printDir(){
   //serves as a transit between the inode_state which can be accessed
   //by commads.cpp and the directory which can access the map where
   //our data is stored
   auto dir=directory_ptr_of(contents);
   dir->print_dirents();
}

void inode_state::printDir(string dirName){
   //check if there was a target(s) specified
   //a lack of targets is represented by an empty string
   if(dirName.compare("")==0){
      cout<<cwd->parent_name()<<endl;
      cwd->printDir();
   }
   else{
      inode_ptr dir=findPath(dirName);
      cout<<dir->parent_name()<<endl;
      dir->printDir();
   }
}
//--------------------------------------------------------------------

/*--------------------------------------------------------------------
 */
void directory::print_dirents_recur(){
   if(!dirents.empty()){
      for(const auto& entry:dirents){
         string name=entry.first;
         inode_ptr file=entry.second;
         cout<<"\t"<<file->get_inode_nr()<<"\t"<<file->getSize()
             <<"\t"<<name<<"\t"<<endl;
      }
      for(const auto& entry:dirents){
         if(entry.second->getType()==DIR_INODE
            &&!(entry.first.compare(".")==0
            ||entry.first.compare("..")==0)){
            entry.second->printDir_recur();
            //cout<<endl;
         }
      }
   }
   else
      throw yshell_exn("This directory is empty\n");
      //cerr<<"This directory is empty"<<endl;
}

void inode::printDir_recur(){
   auto dir=directory_ptr_of(contents);
   if(this->getType()==DIR_INODE)
   cout<<this->parent_name()<<": "<<endl;
   dir->print_dirents_recur();
}


void inode_state::printDir_recur(string dirName){
   if(dirName.compare("")==0){
      //cout<<cwd->parent_name()<<endl;
      cwd->printDir_recur();
   }
   else{
      inode_ptr dir=findPath(dirName);
      //cout<<dir->parent_name()<<endl;
      dir->printDir_recur();
   }
}
//--------------------------------------------------------------------


void inode_state::setPrompt(const string& newPrompt){
   prompt=newPrompt;
}

string inode_state::getPrompt(){
   return prompt;
}

/*--------------------------------------------------------------------
 *
 * mkdir()
 *
 * called by fn_mkdir(), orginates in inode_state.
 * If the path to be created does not begin with '/' (as that could
 * cause "cd" to confuse it for the root directory) then we either
 * create it as a subdirectory of cwd, or we follow the path it lays
 * out from cwd to where the new directory should be created
 */
void directory::insert(string dirname,inode_ptr parent){
   auto dot=pair<string,inode_ptr>(dirname,parent);
   dirents.insert(dot);
}

void inode::insert(string dirname,inode_ptr parent){
   auto dir=directory_ptr_of(contents);
   dir->insert(dirname,parent);
}

void inode_state::insert(string dirname,inode_ptr parent){
   root->insert(dirname,parent);
}
/*
void directory::mkdir(const string& dirname,inode_ptr self){
   if(dirents.find(dirname)==dirents.end()){
      auto dot=pair<string,inode_ptr>(dirname,self);
      dirents.insert(dot);
   }
}*/

void directory::mkdir(const string& dirname,inode_ptr parent){
//inode& directory::mkdir(const string& dirname){//, inode_ptr parent){
   //first we check if the directory we wish to create exists in
   //dirents, if not then we create a new directory node associated
   //with the name we have passed in
   if(dirents.find(dirname)==dirents.end()){
      auto newdir=pair<string,inode_ptr>(dirname,make_shared<inode>(
                                     inode(DIR_INODE)));//,parent)));
      dirents.insert(newdir);
      newdir.second->insert(".",newdir.second);
      newdir.second->insert("..",parent);
   }
   else//otherwise we print an error message
      throw yshell_exn(dirname+": already exists as a directory\n");
      //cerr<<dirname<<": already exists as a directory"<<endl;
}

void inode::mkdir(const string& dirname,inode_ptr parent){
//inode& inode::mkdir(const string& dirname){//,inode_ptr parent){
   //this method serves a bridge from the inode_state, which is
   //accessed by fn_mkdir() and the directory, which stores the map
   //that holds the pointers to directories
   auto dir=directory_ptr_of(contents);
//   inode_ptr me=shared_ptr<this>;
   dir->mkdir(dirname,parent);
   //return dir->mkdir(dirname);//,parent);
}

void inode_state::mkdir(const string& dirname){
//inode& inode_state::mkdir(const string& dirname){
   //first we check if the name of the to-be-created directory could
   //cause trouble for "cd", if it does then error message
   if(dirname.front()=='/')
      throw yshell_exn("cannot create "+dirname+
                       " directory names cannot have a \'/\'; this"+
                       " can cause a condusion with the root node"+
                       " when parsing paths\n");
     //cerr<<"cannot create "<<dirname<<" directory names cannot have a"
     //    <<" \'/\'; this can cause confusion with \"cd\" and how we"<<
      //    " parse directory paths"<<endl;
   //otherwise we parse the path given by the user to see if we need
   //to follow the path,
   //if no path is provided, only a directory name, we create a
   //directory with this name as a subdiretory of the current directory
   //otherwise we create a new string the will give us the path UP TO
   //the name of the new directory, which is created after we follow
   //the path up to that point
   else{
      wordvec destination=split(dirname,"/");
      //the size of the vector destination will tell us whether we need
      //to follow a path to create a new directory or not, if a "/" was
      //input by the user (which would specify a path of directories)
      //then we isolate the path from the directory that will be placed
      //the end of it. After creating the new path, we create the
      //directory
      if(destination.size()>1){
         string path="";
         for(auto itor=destination.begin();itor+1!=destination.end()
                                          ;++itor){
            path+=*itor+"/";
         }
         inode_ptr i=findPath(path);
         i->mkdir(destination.back(),i);
         //return i->mkdir(destination.back());//,i);
      }
      else{
         cwd->mkdir(dirname,cwd);
         //return cwd->mkdir(dirname);//,cwd);
      }
   }
}
//--------------------------------------------------------------------

/*--------------------------------------------------------------------
 *
 * make file()
 *
 */
void inode::writefile(const wordvec& newdata){
   if(type==PLAIN_INODE){
      auto file=plain_file_ptr_of(contents);
      file->writefile(newdata);
   }
   else
      throw yshell_exn(
         "Error: trying to write to directory as if it were a file\n");
}

void directory::mkfile(const string& filename,const wordvec& newdata){
//inode& directory::mkfile(const string& filename,const wordvec&
//                                                      newdata){
   if(dirents.find(filename)==dirents.end()){
      auto newfile=pair<string,inode_ptr>(filename,make_shared<inode>(
                                     inode(PLAIN_INODE)));//,parent)));
      //newfile->writefile(newdata);
      //auto file=plain_file_ptr_of(newfile.second);
      //file.writefile(newdata);
       dirents.insert(newfile);
   }
   //else
        //dirents.at(filename)->writefile(newdata);
   dirents.at(filename)->writefile(newdata);
}

void inode::mkfile(const string& filename,const wordvec& newdata){
//inode& inode::mkfile(const string& filename, const wordvec& newdata){
   auto file_dest=directory_ptr_of(contents);
   file_dest->mkfile(filename,newdata);
}

void inode_state::mkfile(const string& filename,const wordvec& newdata){
//inode& inode_state::mkfile(const string& filename,const wordvec&
//                                                         newdata){
   if(filename.front()=='/'){
      inode_ptr i=root;
      wordvec destination=split(filename,"/");
      string dest=destination.back();
      if(destination.size()>1){
         string path="";
         for(auto itor=destination.begin();itor+1!=destination.end()
                                          ;++itor){
            path+=*itor+"/";
         }
         i=findPath(path);
         return i->mkfile(dest,newdata);
      }
      else
         return root->mkfile(dest,newdata);
   }
   else{
      wordvec destination=split(filename,"/");
      if(destination.size()>1){
         string path="";
         for(auto itor=destination.begin();itor+1!=destination.end()
                                          ;++itor){
            path+=*itor+"/";
         }
         inode_ptr i=findPath(path);
         string dest=destination.back();
         i->mkfile(dest,newdata);
      }
      else{
         cwd->mkfile(filename,newdata);
      }
   }
}

//--------------------------------------------------------------------

//--------------------------------------------------------------------
//remove()
//functions that lead to the calling of the directory.remove()
//which will remove an empty directory or file
void inode::remove(const string& filename){
//cout<<filename<<endl;
   auto dir=directory_ptr_of(contents);
//   inode_ptr me=shared_ptr<this>;
   dir->remove(filename);
}

void inode_state::remove(const string& filename){
   if(filename.compare(".")==0||filename.compare("..")==0)
      throw yshell_exn("cannot remove \".\" or \"..\"\n");
      //cerr<<"cannot remove \".\" or \"..\""<<endl;
   else if(filename.front()=='/'){
      if(filename.length()==1)
         throw yshell_exn("cannot remove the root\n");
         //cerr<<"cannot remove the root"<<endl;
      else{
         inode_ptr i=root;
         wordvec destination=split(filename,"/");
         string dest=destination.back();
         if(destination.size()>1){
            string path="";
            for(auto itor=destination.begin();itor+1!=destination.end()
                                             ;++itor){
               path+=*itor+"/";
            }
            i=findPath(path);
            //if(dest!=cwd)
            i->remove(dest);
         }
         else{
            //if(dest!=cwd)
            root->remove(dest);
         }
      }
   }
   else{
      wordvec destination=split(filename,"/");
      if(destination.size()>1){
         string path="";
         for(auto itor=destination.begin();itor+1!=destination.end()
                                          ;++itor){
            path+=*itor+"/";
         }
         inode_ptr i=findPath(path);
         i->remove(destination.back());//,i);
      }
      else
         cwd->remove(filename);//,cwd);
   }
}
//--------------------------------------------------------------------
/*
 * remove_recursive()
 */
void directory::remove_recur(const string& filename) {
   DEBUGF ('i', filename);
   for(const auto& entry:dirents){
      if(entry.first.compare(".")!=0&&entry.first.compare("..")!=0)
         //entry.second->remove_recur(entry.first);
         entry.second->clean();
   }
   dirents.erase(filename);
}

void inode::remove_recur(const string& filename){
   if(type!=PLAIN_INODE){
      auto dir=directory_ptr_of(contents);
      return dir->remove_recur(filename);
   }
}

void inode_state::remove_recur(const string& filename){
   if(filename.compare(".")==0||filename.compare("..")==0
                              ||filename.compare("/")==0)
      throw yshell_exn("cannot remove \".\" or \"..\" or the root\n");
      //cerr<<"cannot remove \".\" or \"..\""<<endl;
   else if(filename.front()=='/'){
      inode_ptr i=root;
      wordvec destination=split(filename,"/");
      string dest=destination.back();
      if(destination.size()>1){
         string path="";
         for(auto itor=destination.begin();itor+1!=destination.end()
                                          ;++itor){
            path+=*itor+"/";
         }
         i=findPath(path);
         return i->remove_recur(dest);
      }
      else
         return root->remove_recur(dest);
   }
   else{
      wordvec destination=split(filename,"/");
      if(destination.size()>1){
         string path="";
         for(auto itor=destination.begin();itor+1!=destination.end()
                                          ;++itor){
            path+=*itor+"/";
         }
         inode_ptr i=findPath(path);
         return i->remove_recur(destination.back());//,i);
      }
      else
         //cwd->clean();
         return cwd->remove_recur(filename);//,cwd);
   }
}
//-------------------------------------------------------------------- 

void directory::clean(){
//   dirents.clear();
  
   for(const auto& entry:dirents){
      inode_ptr i=entry.second;
      if(entry.first.compare("..")!=0&&entry.first.compare(".")!=0)
         i->clean();
      //entry.second->clean();
      //entry.second=nullptr;
      //dirents.erase(entry.first);
   }
   dirents.clear();
}

void inode::clean(){
   if(type==DIR_INODE){
      auto dir=directory_ptr_of(contents);
      dir->clean();
      contents=nullptr;
   }
   else{
      auto dir=plain_file_ptr_of(contents);
      contents=nullptr;
   }
}

void inode_state::clean(){
   cwd=root;
   root->clean();
   root=nullptr;
}
