#ifndef clox_memory_h
#define clox_memery_h

#include "common.h"
#include "object.h"

// reallocated requires pointer which we give NULL to and old_cap which we give 0 to
// so basically it allocs sizeof(type) * count
#define ALLOCATE(type,count)\
    (type*)reallocate(NULL,0,sizeof(type) * (count))


#define FREE(type,pointer) reallocate(pointer,sizeof(type),0)

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)



// so basically smth like this = relloacte(uint_8 * old_cap / uint_8 * new_cap)
#define GROW_ARRAY(type,pointer,oldCap,new_count) \
    (type*)reallocate(pointer,sizeof(type) * (oldCap), \
    sizeof(type) * (new_count))


// just an abstraction over the reallocate func 
// if the new_size is 0 , realloc will automatically just frees the memorey
#define FREE_ARRAY(type,pointer,cap)\
    reallocate(pointer,sizeof(type)*(cap),0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void freeObjects();


#endif
