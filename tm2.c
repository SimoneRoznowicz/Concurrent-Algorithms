// Requested features
#define _GNU_SOURCE
#define _POSIX_C_SOURCE   200809L
#ifdef __STDC_NO_ATOMICS__
#error Current C11 compiler does not support atomic operations
#endif

// External headers
// #include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h> //TODO: REMOVE FOR DEBUG PURPOSES
#include <semaphore.h>

#if (defined(__i386__) || defined(__x86_64__)) && defined(USE_MM_PAUSE)
#include <xmmintrin.h>
#else

#include <sched.h>

#endif

// Internal headers
#include <tm.h>

// -------------------------------------------------------------------------- //

/** Define a proposition as likely true.
 * @param prop Proposition
**/
#undef likely
#ifdef __GNUC__
#define likely(prop) \
        __builtin_expect((prop) ? 1 : 0, 1)
#else
#define likely(prop) \
        (prop)
#endif

/** Define a proposition as likely false.
 * @param prop Proposition
**/
#undef unlikely
#ifdef __GNUC__
#define unlikely(prop) \
        __builtin_expect((prop) ? 1 : 0, 0)
#else
#define unlikely(prop) \
        (prop)
#endif

/** Define one or several attributes.
 * @param type... Attribute names
**/
#undef as
#ifdef __GNUC__
#define as(type...) \
        __attribute__((type))
#else
#define as(type...)
#warning This compiler has no support for GCC attributes
#endif

// -------------------------------------------------------------------------- //


/** Pause for a very short amount of time.
**/
static inline void pause() {
#if (defined(__i386__) || defined(__x86_64__)) && defined(USE_MM_PAUSE)
    _mm_pause();
#else
    sched_yield();
#endif
}

// -------------------------------------------------------------------------- //

#define DEFAULT_FLAG 0
#define REMOVED_FLAG 1
#define ADDED_FLAG 2
#define ADDED_REMOVED_FLAG 3

#define BATCHER_NB_TX 12ul
#define MULTIPLE_READERS UINTPTR_MAX - BATCHER_NB_TX

static const tx_t read_only_tx = UINTPTR_MAX - 1ul;
static const tx_t destroy_tx = UINTPTR_MAX - 2ul;

struct map_elem{
	size_t size=0;



};

struct region{
	size_t align=0;
	struct batcher batcher;



};

struct batcher{
	atomic_ulong counter;
	atomic_ulong num_entered_proc;
	atomic_ulong num_writing_proc;
	atomic_ulong epoch;
	atomic_ulong acquire_lock;
	atomic_ulong ticket;
};

struct region{
	size_t align;
	struct batcher batcher;
	struct map_elem map_elem;




};

void get_new_batcher((struct batcher)* batcher){
	batcher->counter=0;
	batcher->num_entered_proc=0;
	batcher->num_writing_proc=0;
	batcher->epoch=0;
	batcher->aquire_lock=0;
	batcher->ticket=0;
}

shared_t tm_create(size_t size, size_t align){
	(struct region)* region=(struct region)* malloc(sizeof(region));
	if(unlikely(region==NULL)){
		return invalid_shared;
	}
	region->index=1; //index 0 memory is reserved
	size_t real_align=align>sizeof(void*)?align:sizeof(void*);//boh questo non so... Io terrei anche solo align
	size_t size=real_align*
};









