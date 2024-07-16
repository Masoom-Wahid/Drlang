#ifndef drlang_compiler_h

#define drlang_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction* compile(const char* source,Chunk *chunk);


#endif
