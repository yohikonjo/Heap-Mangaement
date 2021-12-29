/*
   Name: Rafel Tsige
   Id:   1001417200
*/

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)
 
static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */
/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
   /* Implemntation of Best Fit by iterating through the whole heap once and finding the memory block that has a size larger than that of the one requested with the smallest size difference between them that is free */
   int best_fit = INT_MAX;
   struct _block* winner = NULL;
   while (curr) 
   {
      if(curr->size <= best_fit && curr->size >= (int) size && curr->free)
      {
         best_fit = curr->size;
         winner = curr;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = winner;
#endif

#if defined WORST && WORST == 0
   /* Implemntation of Worst Fit by iterating through the whole heap once and finding the memory block that has a size larger than that of the one requested with the largest size difference between them that is free */
   int worst_fit = INT_MIN;
   struct _block* winner = NULL;
   
   while (curr) 
   {
      if(curr->size >= worst_fit && curr->size >= (int) size && curr->free)
      {
         worst_fit = curr->size;
         winner = curr;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = winner;
#endif


#if defined NEXT && NEXT == 0
   /* Implementing Next Fit by iterating the heap from the last visited location and returning the first memory block with a larger size than the size of the memory block requested */
   struct _block *next = NULL;
   
   while (curr) 
   {
      if(curr->size >= (int)size && curr->free)
      {
         next = curr;
      }
      *last = curr;
      curr = curr->next;
   }
   curr = next;
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr; 
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   num_grows++;
   num_blocks++;
   max_heap += size;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;
   num_mallocs++;
   num_requested += size;
   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

void *calloc(size_t blocks, size_t size) 
{
   struct _block *mem = malloc(blocks*size);
   memset(mem, 0, blocks*size);
   return mem;
   
}

void *realloc(void *ptr, size_t size)
{
   struct _block *reall = malloc(size);
   memcpy(reall, ptr, size);
   free(ptr);
   return reall;
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;
   /* Merges two adjacent memory blocks that are both free into one larger memory block this is to increase how efficiently we use our memory by helping to limit the number of unused pockets of memory in our heap */
   struct _block *newer = NULL;
   struct _block *current = heapList; 
   newer = current->next;
   
   while(current)
   {
      if(newer != NULL && current->free && newer->free)
      {
         int new_size = current->size + newer->size + sizeof(struct _block);
         newer = newer->next;
         current->size = new_size;
         current->next = newer;
         
         num_coalesces++;
         num_blocks--;
      }
      current = current->next;
   }
}
/* 7f704d5f-9811-4b91-a918-57c1bb646b70       --------------------------------*/
/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp`: --------------------------------*/
