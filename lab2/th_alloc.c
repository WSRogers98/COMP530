// test for branches
/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

/* Tar Heels Allocator
 *
 * Simple Hoard-style malloc/free implementation.
 * Not suitable for use for large allocatoins, or
 * in multi-threaded programs.
 *
 * to use:
 * $ export LD_PRELOAD=/path/to/th_alloc.so <your command>
 */

/* Hard-code some system parameters */

#define SUPER_BLOCK_SIZE 4096
#define SUPER_BLOCK_MASK (~(SUPER_BLOCK_SIZE-1))
#define MIN_ALLOC 32 /* Smallest real allocation.  Round smaller mallocs up */
#define MAX_ALLOC 2048 /* Fail if anything bigger is attempted.
                        * Challenge: handle big allocations */
#define RESERVE_SUPERBLOCK_THRESHOLD 2

#define FREE_POISON 0xab
#define ALLOC_POISON 0xcd

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define assert(cond) if (!(cond)) __asm__ __volatile__ ("int $3")

/* Object: One return from malloc/input to free. */
struct __attribute__((packed)) object {
    union {
        struct object *next; // For free list (when not in use)
        char * raw; // Actual data
    };
};

/* Super block bookeeping; one per superblock.  "steal" the first
 * object to store this structure
 */
struct __attribute__((packed)) superblock_bookkeeping {
    struct superblock_bookkeeping * next; // next super block
    struct object *free_list;
    // Free count in this superblock
    uint8_t free_count; // Max objects per superblock is 128-1, so a byte is sufficient
    uint8_t level;
};

/* Superblock: a chunk of contiguous virtual memory.
 * Subdivide into allocations of same power-of-two size. */
struct __attribute__((packed)) superblock {
    struct superblock_bookkeeping bkeep;
    void *raw;  // Actual data here
};


/* The structure for one pool of superblocks.
 * One of these per power-of-two */
struct superblock_pool {
    struct superblock_bookkeeping *next;
    uint64_t free_objects; // Total number of free objects across all superblocks
    uint64_t whole_superblocks; // Superblocks with all entries free
};

// 10^5 -- 10^11 == 7 levels
#define LEVELS 7
static struct superblock_pool levels[LEVELS] = {{NULL, 0, 0},//2^5 =32
                                                {NULL, 0, 0},//2^6 = 64
                                                {NULL, 0, 0},//2^7 = 128
                                                {NULL, 0, 0},//2^8 = 256
                                                {NULL, 0, 0},//2^9 = 512
                                                {NULL, 0, 0},//2^10 = 1024
                                                {NULL, 0, 0}};//2^11 =2048

/* Data structure to track large objects (bigger than MAX_ALLOC.
 * We will just do a simple linked-list by default.
 *
 * Extra credit to do something more substantial (or before seeing this code).
 */
struct big_object {
    struct big_object *next;
    void *addr;
    size_t size;
};

// List of big objects
static struct big_object *big_object_list = NULL;

/* Convert the size to the correct power of two.
 * Recall that the 0th entry in levels is really 2^5,
 * the second level represents 2^6, etc.
 *
 * Return the index to the appropriate level (0..6), or
 * -1 if the size is too large.
 */

static inline int size2level (ssize_t size) {
    /* Your code here. */
    // Temporarily suppress the compiler warning that size is unused
   // power: the power of two to store in this superblock.  Note that
     //    this is offset by 5; so a power zero means 2^5.
    if (size <=0 || size >MAX_ALLOC){
        return errno;
    }
    if (size <=MIN_ALLOC){
        return 0;
    }
    if (size>MIN_ALLOC && size<=64){
        return 1;
    }
    if (size>64 && size<=128){
        return 2;
    }
    if (size>128 && size<=256){
        return 3;
    }
    if (size>256 && size<=512){
        return 4;
    }
    if (size>512 && size<=1024){
        return 5;
    }
    if (size>1024 && size<=2048){
        return 6;
    }
     // You should remove the following line
    //   (void)(size);
   // return 0;
    return errno;
}

/* This function allocates and initializes a new superblock.
 *
 * Note that a superblock in this lab is only one 4KiB page, not
 * 8 KiB, as in the hoard paper.
 *
 * This design sacrifices the first entry in every superblock
 * to store a superblock_bookkeeping structure.  Yes,
 * it is a bit wasteful, but let's keep the exercise simple.
 *
 * power: the power of two to store in this superblock.  Note that
 *        this is offset by 5; so a power zero means 2^5.
 *
 * Return value: a struct superblock_bookkeeping, which is
 * embedded at the start of the superblock.  Or NULL on failure.
 */
static inline
struct superblock_bookkeeping * alloc_super (int power) {

    void *page;
    struct superblock* sb;
    int free_objects = 0, bytes_per_object = 0;
    char *cursor;
    // Your code here
    // Allocate a page of anonymous memory
    // WARNING: DO NOT use brk---use mmap, lest you face untold suffering
    page= mmap(NULL,4096,PROT_READ | PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1,0);
    sb = (struct superblock*) page;
    // Put this one the list.
    sb->bkeep.next = levels[power].next;
    levels[power].next = &sb->bkeep;
    levels[power].whole_superblocks++;
    sb->bkeep.level = power;
    sb->bkeep.free_list = NULL;

    // Your code here: Calculate and fill the number of free objects in this superblock
    //  Be sure to add this many objects to levels[power]->free_objects, reserving
    //  the first one for the bookkeeping.
    // Be sure to set free_objects and bytes_per_object to non-zero values.

    bytes_per_object = 1<<(power+5); //power offset i think? note to ask TA if this is correct.
    free_objects = SUPER_BLOCK_SIZE/bytes_per_object -1;
    sb->bkeep.free_count = free_objects;
    levels[power].free_objects = levels[power].free_objects + free_objects;

    // The following loop populates the free list with some atrocious
    // pointer math.  You should not need to change this, provided that you
    // correctly calculate free_objects.

    cursor = (char *) sb;
    // skip the first object
    for (cursor += bytes_per_object; free_objects--; cursor += bytes_per_object) {
        // Place the object on the free list
        struct object* tmp = (struct object *) cursor;
        tmp->next = sb->bkeep.free_list;
        sb->bkeep.free_list = tmp;
    }
    return &sb->bkeep;
}

void *malloc(size_t size) {
    struct superblock_pool *pool;
    struct superblock_bookkeeping *bkeep;
    void *rv = NULL;
    int power = size2level(size);
    int bytes = 1<<(power+5);

    // Handle bigger allocations with mmap, and a simple list
    if (size > MAX_ALLOC) {
        // Why, yes we can do a recursive malloc.  But carefully...
        struct big_object *biggun = malloc(sizeof(struct big_object));
        // nice name
        biggun->next = big_object_list;
        biggun->addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        assert(biggun->addr);
        biggun->size = size;
        big_object_list = biggun;
        return biggun->addr;
    }

    // Delete the following two lines
   // errno = -ENOMEM;
  //  return rv;

    pool = &levels[power];

    if (!pool->free_objects) {
        bkeep = alloc_super(power);
    } else {
        bkeep = pool->next;
    }

    for ( ; bkeep != NULL; bkeep = bkeep->next) {
        if (bkeep->free_count) {
            struct object *cursor = bkeep->free_list;
            /* Remove an object from the free list. */
            // Your code here
            //
            // NB: If you take the first object out of a whole
            //     superblock, decrement levels[power]->whole_superblocks
            // Temporarily suppress the compiler warning that cursor is unused
            // You should remove the following line
          //  (void)(cursor);
            rv=cursor;
            bkeep->free_list = cursor->next;
            pool->free_objects--;
            bkeep->free_count--;
            if(bkeep->free_count == SUPER_BLOCK_SIZE/bytes -2){
            pool->whole_superblocks--;
            }
            break;
        }
        else{
            bkeep= bkeep -> next;
        }
    }

    // assert that rv doesn't end up being NULL at this point
    assert(rv != NULL);

    /* Exercise 3: Poison a newly allocated object to detect init errors.
     * Hint: use ALLOC_POISON
     * 
     */
    memset(rv, ALLOC_POISON, bytes);
    return rv;
}

static inline
struct superblock_bookkeeping * obj2bkeep (void *ptr) {
    uint64_t addr = (uint64_t) ptr;
    addr &= SUPER_BLOCK_MASK;
    return (struct superblock_bookkeeping *) addr;
}

void free(void *ptr) {

    // Just ignore free of a null ptr
    if (ptr == NULL) return;
    
    struct superblock_bookkeeping *bkeep = obj2bkeep(ptr);
    int power = bkeep->level;

    // temporary variable to get bytes for memory poisoning
//riyad you can remove this / replace it with your own variable
    int bytes2 = 1<<(power+5);
  //  (void)(bytes2);
    // We need to check for free of any large objects first.
    {
        struct big_object *tmp, *last;
        for(tmp = big_object_list, last = NULL; tmp; last = tmp, tmp = tmp->next) {
            if (tmp->addr == ptr) {
                // We found it
                // Unmap the object
                munmap(tmp->addr, tmp->size);
                // Fix up the list
                if (!last) {
                    big_object_list = tmp->next;
                } else {
                    last->next = tmp->next;
                }
                // Free the node
                free(tmp);
                return;
            }
        }
    }

    // Your code here.
    //   Be sure to put this back on the free list, and update the
    //   free count.  If you add the final object back to a superblock,
    //   making all objects free, increment whole_superblocks.
   
    //ex2 start
    //bytes per object
    int bytesPobj = 1 << (5 + power);
    //mx amount of objs 
    int maxCap = (SUPER_BLOCK_SIZE / bytesPobj) - 1;

    if(bkeep->free_count == maxCap -1) levels[power].whole_superblocks++;

    //ex2 end for now
    
    struct object *curBlock =  (struct object*) ptr;
    memset(curBlock, FREE_POISON,bytes2);
    /* Exercise 3: Poison a newly freed object to detect use-after-free errors.
     * Hint: use FREE_POISON.
     */

     // need free to implement rest

    while (levels[power].whole_superblocks > RESERVE_SUPERBLOCK_THRESHOLD) {
        // Exercise 4: Your code here
        // Remove a whole superblock from the level
        // Return that superblock to the OS, using mmunmap
	
	// place current block and prev, will be used to set to free block of memory
       struct superblock_bookkeeping* curBk = levels[bkeep->level].next;
       struct superblock_bookkeeping* prevBk;
       while (curBk->free_count != maxCap) { //traverse through list to find the first whole superblock
            prevBk = curBk;
            curBk = curBk->next;
        }

       // skip over curBk, essentially getting it out of the list of blocks
       prevBk->next = curBk->next;
       //now munmap cur getting it unmapped
       if (munmap(curBk, SUPER_BLOCK_SIZE) != 0) errno = -ENOMEM;
       
       //dec free_objects because cur is gone, also dec superBlock because a whole will be taken 
       levels[bkeep->level].free_objects = levels[bkeep->level].free_objects - maxCap;       
       levels[bkeep->level].whole_superblocks--;
	
	
	// break; hack to keep this loop from hanging; remove in ex 4
    }
        // rest of ex2
     // place at beginning of free list
   ((struct object *)ptr)->next = bkeep->free_list;
   bkeep->free_list = ptr;

    //inc free objects after adding to list, same thing for free count
    levels[power].free_objects++;
    bkeep->free_count++;

    //end ex2 for good

}
 
// Do NOT touch this - this will catch any attempt to load this into a multi-threaded app
int pthread_create(void __attribute__((unused)) *x, ...) {
    exit(-ENOSYS);
}
