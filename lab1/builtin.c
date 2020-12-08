/* COMP 530: Tar Heel SHell
 *
 * This file implements a table of builtin commands.
 */
#include "thsh.h"
#include <stdlib.h>

struct builtin {
  const char * cmd;
  int (*func)(char *args[MAX_ARGS], int stdin, int stdout);
};


static char old_path[MAX_INPUT];
static char cur_path[MAX_INPUT];
bool debugger=false;
/* This function needs to be called once at start-up to initialize
 * the current path.  This should populate cur_path.
 *
 * Returns zero on success, -errno on failure.
 */
int init_cwd(void) {

  // Lab 1: Your code here

    // get current directory
  
   
     getcwd(cur_path, MAX_INPUT);
     strcpy(old_path, cur_path);
      
    //char *currdir;
    // pretty sure this is needed to differentiate from the pwd
    //currdir=strcat(dir, "/");
   //strcpy(cur_path, currdir);
   
    return 0;
}

/* Handle a cd command.  */
int handle_cd(char *args[MAX_INPUT], int stdin, int stdout) {

  // Note that you need to handle special arguments, including:
  // "-" switch to the last directory
  // "." switch to the current directory.  This should change the
  //     behavior of a subsequent "cd -"
  // ".." go up one directory
  //
  // Hint: chdir can handle "." and "..", but saving
  //       these results may not be the desired outcome...

  // XXX: Be sure to write test cases for ., .., and weirdness
  // XXX: Test for errors in the output if a cd fails

  // Lab 1: Your code here
  //
  if(debugger==false){
    char dir[MAX_INPUT];
    char *tempstr="-";
   // int run=0;
   if(args[1] !=NULL){
    if(strncmp(args[1], "-",strlen(tempstr))==0){
      strcpy(dir,old_path);
      strcpy(old_path, cur_path);
      strcpy(cur_path,dir);
      if(chdir(dir)==0){
        chdir(dir);
        return 0;
      }else{
        return -errno;
      }
    
    //  run =1;
    }
    else{
      strcpy(dir,args[1]);
         strcpy(old_path, cur_path);
         strcpy(cur_path,dir);
         return chdir(dir);
    //    run =1;
    }
   }
    //for safety
  //    if(run !=1){
  //   return -errno;
  // }
  //   pid_t pid =fork();
  // if(pid <0){
  //   return -errno;
  // }else if (pid ==0){
  
  // return chdir(dir);
  // }else{
  //   return -errno;
  // }
  //  Remove the following two lines once implemented.  These
  // just suppress the compiler warning around an unused variable
  //(void) cur_path;
  //(void) old_path;

  return 42;
  } 
  if(debugger==true){
    fprintf(stderr, "RUNNING: [cd]\n");
     char dir[MAX_INPUT];
    char *tempstr="-";
   // int run=0;
   if(args[1] !=NULL){
    if(strncmp(args[1], "-",strlen(tempstr))==0){
      strcpy(dir,old_path);
      strcpy(old_path, cur_path);
      strcpy(cur_path,dir);
      if(chdir(dir)==0){
        chdir(dir);
         fprintf(stderr, "ENDED: [cd] (ret=0)\n");
        return 0;
      }else{
         fprintf(stderr, "ENDED: [cd] (ret=%d)\n",-errno);
        return -errno;
      }
    
    //  run =1;
    }
    else{
      int rv=0;
      strcpy(dir,args[1]);
         strcpy(old_path, cur_path);
         strcpy(cur_path,dir);
         rv=chdir(dir);
        fprintf(stderr, "ENDED: [cd] (ret=%d)\n",rv);
         return chdir(dir);
    //    run =1;
    }
   }
return 42;
  }
  return 42;
}

/* Handle an exit command. */
int handle_exit(char *args[MAX_ARGS], int stdin, int stdout) {
  exit(0);
  return 0; // Does not actually return
}

int handle_goheels(char *args[MAX_ARGS], int stdin, int stdout){
  int temp;
  FILE *txt;
  txt = fopen("goheels.txt","r");
  while((temp=getc(txt))!= EOF){
      putchar(temp);
  }
  fclose(txt);
  return 0;
}


static struct builtin builtins[] = {{"cd", handle_cd},
				    {"exit", handle_exit},
            {"goheels", handle_goheels},
				    {'\0', NULL}};

/* This function checks if the command (args[0]) is a built-in.
 * If so, call the appropriate handler, and return 1.
 * If not, return 0.
 *
 * stdin and stdout are the file handles for standard in and standard out,
 * respectively. These may or may not be used by individual builtin commands.
 *
 * Places the return value of the command in *retval.
 *
 * stdin and stdout should not be closed by this command.
 *
 * In the case of "exit", this function will not return.
 */
int handle_builtin(char *args[MAX_ARGS], int stdin, int stdout, int *retval, bool debug) {
  int rv = 0;
  debugger=debug;
  // Lab 0: Your Code Here
  // Comment this line once implemented.  This just suppresses
  // the unused variable warning from the compiler.
  if (strcmp(args[0], "cd")==0){
    //handle_cd(args,stdin,stdout);
    rv=1;
    *retval=handle_cd(args,stdin,stdout);
      return rv;
  }
  else if (strcmp(args[0], "exit")==0){
   // handle_exit(args,stdin,stdout);
    rv=1;
   *retval= handle_exit(args,stdin,stdout);
    return rv;
  }else if (strcmp(args[0], "goheels")==0){
   // handle_goheels(args,stdin,stdout);
    rv=1;
   *retval= handle_goheels(args,stdin,stdout);
    return rv;
  }
  else{
    *retval=rv;
    (void) builtins;
  return rv;
  }
}

/* This function initially prints a default prompt of:
 * thsh>
 *
 * In Lab 1, Exercise 3, you will add the current working
 * directory to the prompt.  As in, if you are in "/home/foo"
 * the prompt should be:
 * [/home/foo] thsh>
 *
 * Returns the number of bytes written
 */
int print_prompt(void) {
  int ret = 0;
  char *cwd;
  cwd=getcwd(cur_path, MAX_INPUT);
  // Print the prompt
  // file descriptor 1 -> writing to stdout
  // print the whole prompt string (write number of
  // bytes/chars equal to the length of prompt)
  //
  const char *prompt = " thsh> ";
    write(1, "[", strlen("["));
    write(1, cwd, strlen(cwd));
    write(1, "]", strlen("]"));
  // Lab 1: Your code here


  ret = write(1, prompt, strlen(prompt));
  return ret;
}
