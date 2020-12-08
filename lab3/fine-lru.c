/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

/* @* Place your name here, and any other comments *@
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
    // Protects this node's contents
    pthread_mutex_t mutex;
};

static struct list_node* list_head = NULL;

/* A static mutex; protects the count and head.
 * XXX: We will have to tolerate some lag in updating the count to avoid
 * deadlock. */
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
  //  (void)list_head;
  //  (void)mutex;
   // (void)count;
  //  (void)cv_low;
  //  (void)cv_high;
    /* End temporary code */
    printf("init\n");
    int temp1= pthread_mutex_init(&mutex, NULL);
    printf("1: %d\n",temp1);
    int temp2= pthread_cond_init(&cv_low, NULL);
    printf("2: %d\n",temp2);
    int temp3= pthread_cond_init(&cv_high, NULL);
    printf("3: %d\n",temp3);
    if( temp1!=0 || temp2!=0 || temp3!=0){
      printf("failed\n");
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
    return 0;
}

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
     while(count >HIGH_WATER_MARK){
        pthread_cond_wait(&cv_high, &mutex);
    }
    return 1;
}

/* Do a pass through all elements, either decrement the reference count,
 * or remove if it hasn't been referenced since last cleaning pass.
 *
 * check_water_mark: If 1, block until there are more elements in the cache
 * than the LOW_WATER_MARK.  This should only be 0 during self-testing or in
 * single-threaded mode.
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
    if(check_water_mark==1){
        while(count<=LOW_WATER_MARK){
        pthread_cond_wait(&cv_low, &mutex);
        }
    }
}


/* Optional shut-down routine to wake up blocked threads.
   May not be required. */
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
