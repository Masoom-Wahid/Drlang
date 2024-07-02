#ifndef clox_memory_h
#define clox_memery_h

#include "common.h"

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

#endif
