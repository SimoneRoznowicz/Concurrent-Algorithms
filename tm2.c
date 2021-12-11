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

#define DEFAULT 0
#define REMOVED 1
#define ADDED 2
#define ADDED_REMOVED 3

#define BATCHER_NB_TX 12ul
#define MULTIPLE_READERS UINTPTR_MAX - BATCHER_NB_TX

static const tx_t read_only_tx = UINTPTR_MAX - 1ul;
static const tx_t destroy_tx = UINTPTR_MAX - 2ul;

struct map_elem{
	size_t size=0;
	void* ptr;



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
	atomic_ulong attempt;
};

struct region{
	size_t align;
	size_t align_real;
	struct batcher batcher;
	(struct map_elem)* map_elem;	//array of map_elem
	atomic_ulong  index;




};

void get_new_batcher((struct batcher)* batcher){
	batcher->counter=0;
	batcher->num_entered_proc=0;
	batcher->num_writing_proc=0;
	batcher->epoch=0;
	batcher->aquire_lock=0;
	batcher->permission=0;
}

shared_t tm_create(size_t size, size_t align){
	(struct region)* region=(struct region)* malloc(sizeof(region));
	if(unlikely(region==NULL)){
		return invalid_shared;
	}
	region->index=1; //index 0 memory isi reserved
	region->map_elem=malloc(getpagesize());
	memset(region->map_elem,0,getpagesize());
	if(unlikely(region->map_elem==NULL)){
		return invalid_shared;
	}
	size_t align_real=align > sizeof(void*)? align : sizeof(void*);//boh questo non so... Io terrei anche solo align

	//initialize the remaining values of region->map_elem->...
	region->map_elem->size=size;
	region->map_elem->my_status=0;
	region->map_elem->status=DEFAULT;
	region->align_real=align_real;
	region->align=align;
	get_new_batcher(&(region->batcher));
	return region;
};

void tm_destroy(shared_t shared) {
	(struct region)* region=(struct region*) shared;
	for(int i=0;i<region->map_elem;i++){
		free(region->map_elem[i].ptr);
	}
}

void *tm_start(shared_t shared){
    return (((struct region)*)shared)->map_elem->ptr;
}

size_t tm_size(shared_t shared){
    return (((struct region)*)shared)->map_elem->size;
}

size_t tm_align(shared_t shared){
    return (((struct region)*)shared)->align_real;
}

tx_t tm_begin(shared_t shared,bool is_ro){
    return enter(&((((struct region)*)shared)->batcher), is_ro);
}

bool tm_end(shared_t shared,tx_t tx){
    leave(&(((struct region)*)shared)->batcher,((struct region)*)shared,tx);
    return true;
}

bool tm_free(shared_t shared,tx_t tx,void *segment){
    struct map_elem* map_elem = get_segment(((struct region)*)shared,segment);
    tx_t previous = 0;
    if (mapping == NULL || !(atomic_compare_exchange_strong(&mapping->status_owner,&previous,tx) || previous == tx)) {
        tm_rollback(((struct region)*)shared,tx);
        return false; //need to roolback since the transaction was not committed 
    }
    if (map_elem->status == ADDED) {
        map_elem->>status = ADDED_REMOVED;
    }
    else {
        map_elem->status = REMOVED;
    }
    return true; //the transaction finished positively, can go on
}

tx_t enter(struct batcher* batcher,bool is_ro){
	if(is_ro){
		unsigned long attempt=atomic_fetch_add_explicit(&(batcher->permission),1,memory_order_relaxed);/*On a multi-core/multi-CPU systems, with plenty of locks that are held for a very short amount 
of time only, the time wasted for constantly putting threads to sleep and waking them up again 
might decrease runtime performance noticeably. When using spinlocks instead, threads get the 
chance to take advantage of their full runtime quantum
		*/
		//keep iterating until the value of the obtained attempt corresponds to the pass
		while(atomic_load_explicit()!=){
			//PAUSE FOR SOME INSTANTS
		}
		//beginning, acquire
		atomic_thread_fence(memory_order_acquire);
		atomic_fetch_add_explicit(&(batcher->num_entered_proc),1,memory_order_relaxed);
		atomic_fetch_add_explicit(&(batcher->pass),1,memory_order_relaxed);
		//end, release
		return read_only_tx;
	}
	else{ //one or more processes write
		while (true) {
            		unsigned long attempt = atomic_fetch_add_explicit(&(batcher->),1, memory_order_relaxed);
            		//spinning locks again
			while (atomic_load_explicit(&(batcher->pass),memory_order_relaxed) != ticket)
                		pause();
            		atomic_thread_fence(memory_order_acquire);
			if (atomic_load_explicit(&(batcher->counter),memory_order_relaxed) == 0) {
                	unsigned long int epoch = atomic_load_explicit(&(batcher->epoch), memory_order_relaxed);
                	atomic_fetch_add_explicit(&(batcher->pass),1,memory_order_release);
			//spinning locks again
                	while (atomic_load_explicit(&(batcher->epoch),memory_order_relaxed) == epoch)
                    		pause();
                	atomic_thread_fence(memory_order_acquire);
			}
		       	else {
                		atomic_fetch_add_explicit(&(batcher->counter),-1,memory_order_release);
                		break;
            		}
		}
		atomic_fetch_add_explicit(&(batcher->nb_entered),1,memory_order_relaxed);
        	atomic_fetch_add_explicit(&(batcher->pass),1,memory_order_release);
        	tx_t tx = atomic_fetch_add_explicit(&(batcher->nb_write_tx),1,memory_order_relaxed) + 1;
        	atomic_thread_fence(memory_order_release);
		print("Inside enter, not ro: tx == ");
		print(tx);
        	return tx;		//return the number of the transaction
	}	
}

void leave((struct batcher)* batcher, (struct region)* region, tx_t tx) {
	unsigned long attempt=atomic_fetch_add_explicit(&(batcher->permission),1,memory_order_relaxed);
        //keep iterating until the value of the obtained attempt corresponds to the pass
        while(atomic_load_explicit()!=){
                //PAUSE FOR SOME INSTANTS
	}
	//beginning, acquire
	atomic_thread_fence(memory_order_acquire);
	//if I have at least one writing operarion inside the batcher
	if(aromic_fetch_add_explicit(&batcher->num_entered_proc,-1,memory_order_relaxed)==1){
		if(atomic_load_explicit(&(batcher->num_writing_proc),memory_order_relaxed)>0)
			commit(region); //commit the operation and add 1 epoch(one operation concluded)
			atomic_fetch_add_explicit(&(batcher->epoch),1,memory_order_relaxed);
			//restore initial values
			atomic_store_explicit(&(batcher->nb_write_tx),0,memory_order_relaxed);
        	    	atomic_store_explicit(&(batcher->counter),BATCHER_NB_TX, memory_order_relaxed);
		}
		atomic_fetch_add_explicit(&(batcher->pass),1,memory_order_release);
	}
	else if(tx!=read_only_tx){
		unsigned long int epoch = atomic_load_explicit(&(batcher->epoch), memory_order_relaxed);
        	atomic_fetch_add_explicit(&(batcher->pass), 1ul, memory_order_release);

        	while (atomic_load_explicit(&(batcher->epoch), memory_order_relaxed) == epoch)
            		pause();
    		} 
		else {
        		atomic_fetch_add_explicit(&(batcher->pass), 1ul, memory_order_release);
    		}
	}
}





