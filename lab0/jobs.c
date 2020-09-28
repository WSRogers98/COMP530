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
pathtemp= malloc(sizeof(char)*(sizeof(getenv("PATH"))));
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

  path_table=malloc(totalpaths*sizeof(char*));
  //handle empty path
  if ((sizeof(pathtemp)==0)){
  path_table[0]='\0';
  return 0;
  }
  char *currpath;
    currpath=malloc(totalchars*sizeof(char));
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
     //ignore empty prefix denoted by ::
   if((pathtemp[i]==':' || pathtemp[i+1]=='\0'))  {
        onpath = false;
       if((pathtemp[i-1] !=':')){
         //path_table[pathcount]=currpath;
         path_table[pathcount]=malloc(sizeof(char)*sizeof(currpath));
         strncpy(path_table[pathcount],currpath,size);
        pathcount++;
       }
        //clear previous path
        currpath=NULL;
        count=0;
        size=1;
        currpath=malloc(totalchars*sizeof(char));
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

