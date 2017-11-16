// $Id: commands.cpp,v 1.12 2015-07-12 04:51:51-07 - - $

#include "commands.h"
#include "debug.h"

/*
 * prints an error if the incorrect number of args are specified for
 * a command (ie: if the user enters % mkdir without any targets
 *
 * all methods(that call this) must have at least one command and one 
 * target so words must be of size 2 or greater. If this is not 
 * fullfilled then an error message is printed.
 * if the error message is printed, a number is returned which will
 * terminate the method which called argsErr()
 */
int argsErr(const wordvec& words){
   if(words.size()<2){
      cerr<<"ERROR: the command "<<words.at(0)
          <<" must have at least 1 target "<<endl;
      return 0;
   }
   return 1;
}

/*
 * when the user specifies a path to a particular file or directory
 * this method is called to a) check that the 
 */
//inode_ptr findPath(inode dir,const wordvec& path){
   /*
   string checkDir=words.front();
   words.erase(0);
   for(auto itor:dir.dirents){
      if(checkDir==itor.first){
         if(itor.second.instanceof(directory)){
            if(words.size()==0)
               return itor.second;
            else
               return findPath(itor.second,path);
         }
         else if(words.size()!=0)
            cerr<<"file or directory not found"<<endl;
         else
            return itor.first;
      }
   }
   return nullptr;
   */
//}

commands::commands(): map ({
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
}){}

command_fn commands::at (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   command_map::const_iterator result = map.find (cmd);
   if (result == map.end()) {
      throw yshell_exn (cmd + ": no such function");
   }
   return result->second;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(argsErr(words)==1){
      wordvec output;
      auto itor=words.begin();
      for(itor++;itor!=words.end();++itor){
         output=state.cat(*itor);
         for(auto itor=output.begin();itor!=output.end();++itor)
            cout<<*itor<<" ";
         cout<<endl;
      }
   }
   //cout<<"This is gonna be hard"<<endl;
}

/*
 * changes the current working directory. This is done by passing a
 * particular string to the method setCwd in inode_state
 *
 * if no arguement is given, the directory is set to root
 * otherwise the string is parsed on "/" with each string representing
 * a directory, the setCwd method will call the findPath method to
 * follow each of these intermidate directories until it reaches the
 * end or finds that some directory doesn't exist and prints an error 
 * message.
 */
void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()==1)
      state.setCwd("");//if setCwd() is passed an empty string,
                       //the current directory will be set to root
   else
      state.setCwd(words.at(1));//otherwise we will follow the path
}

/*
 * method to print from user's input to cout
 * input is taken from words, everything but the command "echo"
 *
 * use an iterator to print every element in words
 * cheat so that the iterator starts printing from words.at(1)
 * instead of words.at(0), thereby skipping "echo"
 * use a check to prevent printing an extra whitespace on the end
 * print a newline after everything so the next user prompt is 
 * easy to see
 */
void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   auto itor=words.begin();
   for(itor++;itor!=words.end();++itor){
      cout<<*itor;
      if(itor+1!=words.end())
         cout<<" ";
   }
   cout<<endl;
}

/*
 * method to exit the shell
 * can accept a custom error status (arguement passed via words) from 
 * user, otherwise defaults to 0 on successful exit
 *
 * if words includes arguements (re: .size()>1)
 * then we must check if the arguement is a valid error status
 */
void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()>1){//check if user provides custion exit status
   /*
 * check if the user provies a valid exit status by iterating over
 * their arguement, if any non-numerical characters are found
 * it is invalid and the status defaults to 127
 */
      int numericalStatus=0;
      for(auto itor = words.at(1).begin();itor!=words.at(1).end();
                                          ++itor){
         if(*itor!='9'&&*itor!='8'&&*itor!='7'&&*itor!='6'&&*itor!='5'
            &&*itor!='4'&&*itor!='3'&&*itor!='2'&&*itor!='1'
            &&*itor!='0'){
            exit_status::set(127);
            numericalStatus=1;//break;
         }
      }
      if(numericalStatus==0){
         int stat=stoi(words.at(1)); 
         exit_status::set(stat);
      }
   }

   throw ysh_exit_exn();//throw and exception to exit the shell
}

/*
 * prints the contents of directories
 *
 * if no target is specified, the contents of the current directory are
 * printed, otherwise all targeted directories are printed in the order
 * they were entered into the terminal
 */
void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   //if there are not targeted directories there will be exactly one
   //string in words
   if(words.size()<2){
//      fn_pwd(state,words);
      state.printDir("");
   }
   //we have to skip over the first value in words because it contains
   //the "ls" command, after that all the targets have their
   //directories printed
   else{
      auto dir=words.begin();
      for(++dir;dir!=words.end();++dir){
//         cout<<*dir<<endl;
         state.printDir(*dir);
      }
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()<2){
//      fn_pwd(state,words);
      state.printDir_recur("");
   }
   else{
      auto dir=words.begin();
      for(++dir;dir!=words.end();++dir){
         state.printDir_recur(*dir);
      }
   }
}


void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(argsErr(words)==1){
      //auto itor=words.begin();
      //for(itor++;itor!=words.end();++itor)
      string filename=words.at(1);
      wordvec writing;
      auto itor=words.begin();
      itor+=2;
      for(;itor!=words.end();++itor)
         writing.push_back(*itor);
      state.mkfile(filename,writing);
   }
}

/*
 * Creates new directories
 *
 * If the name of the to-be-created directory is not specified, an
 * error is printed. Otherwise the name is passed to methods in
 * inode.cpp that will create the directory(s)
 */
void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(argsErr(words)==1){
      auto itor=words.begin();
      for(itor++;itor!=words.end();++itor)
         state.mkdir(*itor);
   }
}

/*
 * method which changes the prompt to the user input
 * similar to echo
 *
 * takes the user input, inserting whitespaces, and skipping
 * the first command like echo.
 * But instead of printing to cout, this is stored in a <string>
 * and it becomes the value of inode_state state.prompt
 * 
 * the final change I made was to add a final space to the prompt
 * unlike echo we want that extra whitespace as buffer
 */
void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string newPrompt {""};
   auto itor=words.begin();
   for(itor++;itor!=words.end();++itor){
      newPrompt.append(*itor);
      newPrompt.append(" ");
   }
   state.setPrompt(newPrompt);
}

/*
 * print the name of the current directory
 *
 * in order to access this information we need to go into indoe_state
 * and its associated classes
 * go to inode.cpp and find string inode_state::cwd_name() for the 
 * continuation of this function
 */
void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout<<state.cwd_name()<<endl;//call inode_state
}

/*
 * removes a file or directory from a directory
 *
 * if more than one target file or directories are specified
 * then all are removed one by one
 */
void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(argsErr(words)==1){
      auto itor=words.begin();
      for(itor++;itor!=words.end();itor++){
         //if(*itor.compare("/")!=0&&*itor.compare("..")!=0
         //                        &&*itor.compare("."))
         state.remove(*itor);
      }
   }
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(argsErr(words)==1){
      auto itor=words.begin();
      for(itor++;itor!=words.end();itor++){
         //if(*itor.compare("/")!=0&&*itor.compare("..")!=0
         //                        &&*itor.compare("."))
         state.remove_recur(*itor);
      }
   }
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}
