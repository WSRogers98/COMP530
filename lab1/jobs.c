/* COMP 530: Tar Heel SHell
 *
 * This file implements functions related to launching
 * jobs and job control.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "thsh.h"

static char ** path_table;

/* Initialize the table of PATH prefixes.
 *
 * Split the result on the parenteses, and
 * remove any trailing '/' characters.
 * The last entry should be a NULL character.
 *
 * For instance, if one's PATH environment variable is:
 *  /bin:/sbin///
 *
 * Then path_table should be:
 *  path_table[0] = "/bin"
 *  path_table[1] = "/sbin"
 *  path_table[2] = '\0'
 *
 * Hint: take a look at getenv().  If you use getenv, do NOT
 *       modify the resulting string directly, but use
 *       malloc() or another function to allocate space and copy.
 *
 * Returns 0 on success, -errno on failure.
 */
int init_path(void) {
  /* Lab 0: Your code here */

  // initiate modifiable Paths and getenv
char *pathtemp;
pathtemp=NULL;
pathtemp= calloc((sizeof(getenv("PATH"))),sizeof(char));
char *envtemp=getenv("PATH");
 pathtemp = envtemp;

int pathcount=0;
 bool onpath=false;

 //start with size enough for 1 / char
 int size=1;


 //find initial table size
 int totalpaths=0;
int totalchars=0;
 for(int i=0; i< strlen(pathtemp); i++){
   if (pathtemp[i]==':'){
     totalpaths+=1;
   }
totalchars+=1;
 }

  path_table=calloc((totalpaths+2),sizeof(char*));
  //handle empty path
  if ((sizeof(pathtemp)==0)){
  path_table[0]='\0';
  return 0;
  }
  char *currpath;
  currpath=NULL;
  currpath=calloc((totalchars+1),sizeof(char));
  // start search for prefixes
 int count =0;
  for(int i=0; i< strlen(pathtemp); i++){
   
   
    // start of first/new prefix
     if ((pathtemp[i]=='/' && onpath==false)) {
      onpath=true;
      currpath[count]= '/';
      count++;
      size++;
     
     }

     //ensure no trasiling slashes & still on same prefix
     if(pathtemp[i] !=':'){
       if((pathtemp[i]=='/' && pathtemp[i+1] !='/' && pathtemp[i+1]!=':' &&currpath[count-1]!='/' &&pathtemp[i-1]!='/' &&pathtemp[i+1]!='\0')||pathtemp[i]!='/'){
        currpath[count]=  pathtemp[i];
        count++;
        size++;
       }
     }

     // end current prefix to begin looking for next prefix 
     //ignore empty prefix denoted by :
   if((pathtemp[i]==':' || pathtemp[i+1]=='\0'))  {
        onpath = false;
       if((pathtemp[i-1] !=':')){
         //path_table[pathcount]=currpath;
         currpath[count+1]='\0';
         path_table[pathcount]=calloc((strlen(currpath)+1),sizeof(char));
         strcpy(path_table[pathcount],currpath);
        pathcount++;
       }
        //clear previous path
        currpath=NULL;
        count=0;
        size=1;
        currpath=calloc((totalchars+1),sizeof(char));
   }
  }
  //end path table with a null
  path_table[pathcount]= '\0';
return 0;
}

/* Debug helper function that just prints
 * the path table out.
 */
void print_path_table() {
  if (path_table == NULL) {
    printf("XXXXXXX Path Table Not Initialized XXXXX\n");
    return;
  }

  printf("===== Begin Path Table =====\n");
  for (int i = 0; path_table[i]; i++) {
    printf("Prefix %2d: [%s]\n", i, path_table[i]);
  }
  printf("===== End Path Table =====\n");
}

/* Given the command listed in args,
 * try to execute it.
 *
 * If the first argument starts with a '.'
 * or a '/', it is an absolute path and can
 * execute as-is.
 *
 * Otherwise, search each prefix in the path_table
 * in order to find the path to the binary.
 *
 * Then fork a child and pass the path and the additional arguments
 * to execve() in the child.  Wait for exeuction to complete
 * before returning.
 *
 * stdin is a file handle to be used for standard in.
 * stdout is a file handle to be used for standard out.
 *
 * If stdin and stdout are not 0 and 1, respectively, they will be
 * closed in the parent process before this function returns.
 *
 * wait, if true, indicates that the parent should wait on the child to finish
 *
 * Returns 0 on success, -errno on failure
 */
int run_command(char *args[MAX_ARGS], int stdin, int stdout, bool wait, bool debug) {
  /* Lab 1: Your code here */
    int rv = 0;
  
    int run=0;
  const char *tempstr=".";
   char *pathtemp2=NULL;

   if(strcmp(args[0],"")== 0){
     return -errno;
   }
   if(debug==true){
   fprintf(stderr, "RUNNING: [%s]\n",args[0]);
   }
  if(strncmp(args[0],".",strlen(tempstr))==0 ||strncmp(args[0],"/",strlen(tempstr))==0){
   // execvp(args[0],args);
   // rv= execvp(args[0],args);
   // return rv;
   pathtemp2=NULL;
   pathtemp2=calloc((strlen(args[0])+4),sizeof(char));
   strcpy(pathtemp2,args[0]);
   run=1;
  }else{
  //  char *temp;
   // for(int i =0; sizeof(args); i++){
   //   temp = args[i];
   char *temp2;
   temp2=NULL;
    temp2=calloc((strlen(args[0])+15),sizeof(char));
    strcpy(temp2, "/");
   strcat(temp2,args[0]);

    struct stat stats;
      for(int j=0; j<sizeof(path_table); j++){
        pathtemp2=NULL;
        pathtemp2=calloc(((strlen(path_table[j]+1))+(strlen(args[0])+1)+15),sizeof(char));
        pathtemp2=strcpy(pathtemp2,path_table[j]);
        strcat(pathtemp2,temp2);
    
          if(stat(pathtemp2,&stats)==0){
            run=1;
            break;
          }
    //  }
    }
  }
  
  if(run !=1){
    rv=-errno; 
    if(debug==true){
   
    fprintf(stderr, "ENDED: [%s] (ret=%d)\n",args[0],rv);
    }
    return rv;
  }
  pid_t pid =fork();
  if(pid <0){
    rv=-errno; 
    if(debug==true){
    fprintf(stderr, "ENDED: [%s] (ret=%d)\n",args[0],rv);
    }
    return rv;
  }else if (pid ==0){
    execvp(pathtemp2, args);
    exit(0);
  
  }else{
    if(wait){
      waitpid(pid,&rv,0);
    }
  }
  if(debug==true){
  fprintf(stderr, "ENDED: [%s] (ret=%d)\n",args[0],0);
  }
  return 0;
}


