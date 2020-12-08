/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

/* @* Place your name here, and any other comments *@
William Rogers
I am so mentally destroyed from this semester that i cant look at code without getting super anxious
I have already pass/failed this class and I pray for a curve
 * @* that deanonymize your work inside this syntax *@
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "lru.h"

/* Define the simple, singly-linked list we are going to use for tracking lru */
struct list_node {
    struct list_node* next;
    int key;
    int refcount;
};

static struct list_node* list_head = NULL;

/* A static mutex */
static pthread_mutex_t mutex;
static int count = 0;
static pthread_cond_t cv_low, cv_high;

static volatile int done = 0;

/* Initialize the mutex. */
int init (int numthreads) {
    /* Your code here */

    /* Temporary code to suppress compiler warnings about unused variables.
     * You should remove the following lines once this file is implemented.
     */
   // (void)list_head;
  // (void)mutex;
  //  (void)count;
  //  (void)cv_low;
 //  (void)cv_high;'
 //future use of temp not needed
 //god i hate this assignment 
 //these should not always equal zero but they do????
 printf("init\n");
 int temp1= pthread_mutex_init(&mutex, NULL);
printf("1: %d\n",temp1);
 int temp2= pthread_cond_init(&cv_low, NULL);
printf("2: %d\n",temp2);
 int temp3= pthread_cond_init(&cv_high, NULL);
 printf("3: %d\n",temp3);
    if( temp1!=0 || temp2!=0 || temp3!=0){
      printf("failed\n");
      // is this ineffecient? yes so idk why i chose to fix it this way then type this but i did
      //return errno
      if( temp1!=0){
          return temp1;
      }
      if( temp2!=0){
          return temp2;
      }
      if( temp3!=0){
          return temp3;
      }
    
    }
    // does it work though, does it really?
    printf("worked\n");
    /* End temporary code */
    //succ
    return 0;
}

// NOTE: return 1 on success and 0 on failure may be for reference and not intialize too. swap 1 and 0 returns above if so
// FINAL NOTE: went with 0 on success errno/1 on failure.
/* Return 1 on success, 0 on failure.
 * Should set the reference count up by one if found; add if not.*/
int reference (int key) {
    /* Your code here */
    pthread_mutex_lock(&mutex);
      if(done){
	pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&cv_low);
    pthread_cond_signal(&cv_high);
    
 	return 0;
    }
    //NOTE: sub > for >= if failure may need to check that part too
    //ex2 wait if high
    while(count >HIGH_WATER_MARK){
        pthread_cond_wait(&cv_high, &mutex);
    }

    //I've been doing too much python and im going crazy
    //bool found =false;
    //why am i talking to myself in my own comments is this how far ive fallen this quarantine??
   struct list_node* c = list_head;
   struct list_node* prev = NULL;
   int found =0;
   while(c){
       if(c->key==key){
           c->refcount++;
           found=1;
           break;
       }
        else if(c->key<key){
           prev=c;
           c=c->next;
       }else{
           break;
       }
   }
   if (found==0){
    struct list_node* node = malloc(sizeof(struct list_node));
    node->key =key;
    node->refcount=1;
    node->next=c;
    count+=1;
    if(prev !=NULL){
        prev->next  =node;
    }else{
        list_head=node;
    }
   }
    if(count>=LOW_WATER_MARK){
    pthread_cond_signal(&cv_low);
    }
    pthread_mutex_unlock(&mutex);
    return 1;
}

/* Do a pass through all elements, either decrement the reference count,
 * or remove if it hasn't been referenced since last cleaning pass.
 *
 * check_water_mark: If 1, block until there are more elements in the cache
 * than the LOW_WATER_MARK.  This should only be 0 during self-testing or in
 * single-threaded mode.
 */
/*
Keeping a Target Cache Size
Ideally, we want to maintain that the cache has a certain number of elements --- i.e., that the count variable stays between LOW_WATER_MARK and HIGH_WATER_MARK. In the second exercise, you will need to use condition variables to ensure two things:

That a call to reference() blocks until count is below HIGH_WATER_MARK, as reference may add an element to the list.
That a call to clean() blocks until count is above LOW_WATER_MARK, ensuring the list doesn't get too small.
That any blocked threads will exit from their current routine when shutdown_threads() is called.
Although you could simply have a thread run in an infinite while loop, we can do better. In particular, we would like you to use condition variables (e.g., pthread_cond_wait() and pthread_cond_signal), as well as the existing mutex synchronization, to allow the delete thread to sleep until there is work to do, and then to wake up once there is.

You will need to ensure that the resulting mutex LRU list implementations supports this behavior correctly. It is not necessary for this to work with the sequential version.
*/
void clean(int check_water_mark) {
    /* Your code here */
    pthread_mutex_lock(&mutex);
      if(done){
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cv_low);
        pthread_cond_signal(&cv_high);
        return;
    }
      //ex2 wait if low
    if(check_water_mark==1){
        while(count<=LOW_WATER_MARK){
        pthread_cond_wait(&cv_low, &mutex);
        }
    }
 
    struct list_node* c=list_head;
    struct list_node* prev=NULL;
    //lab 2 has scarred me so bad i hate pointers
    //mark my words ill never code in C again after this class
    while(c){
        c->refcount--;
        if(c->refcount==0){
        struct list_node* node2=c;
        if(prev){
            prev->next=c->next;
        }else{
            list_head=c->next;
        }
        node2=c->next;
        free(c);
        c=node2;
        count--;
        }else{
            prev=c;
            c=c->next;
        }
    }
    if(count<=HIGH_WATER_MARK){
    pthread_cond_signal(&cv_high);
    }
    pthread_mutex_unlock(&mutex);
}

/* Optional shut-down routine to wake up blocked threads.
   May not be required. */
   //maybe im too tired but i thought shutdown meant remove blocked threads until i read the comment again
void shutdown_threads (void) {
    /* Your code here */
    //end threads
    done=1;
    //signal threads
    pthread_cond_signal(&cv_low);
    pthread_cond_signal(&cv_high);
    return;
}

/* Print the contents of the list.  Mostly useful for debugging. */
void print (void) {
    /* Your code here */
    //following the print example from the assignment page
    printf("=== starting list print ===\n");
    printf("=== total count is %d ===\n",count);
    struct list_node* c = list_head;
    while(c){
        printf ("key %d, ref count %d\n", c->key, c->refcount);
        c=c->next;
    }
    printf("=== ending list print ===");
    pthread_mutex_unlock(&mutex);
}
