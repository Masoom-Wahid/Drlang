#include<stdlib.h>
#include "memory.h"
#include "chunk.h"
#include "object.h"
#include "vm.h"

void* reallocate(void* pointer,size_t old_size,size_t new_size){
    if(new_size == 0){
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer,new_size);
    if (result == NULL) exit(1);
    return result;
}

static void freeObject(Obj* object){
    switch(object->type){
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(char,string->chars,string->length+1);
            FREE(ObjString,object);
            break;
        }
        case OBJ_FUNCTION:
            ObjFunction* function = (ObjFunction*)object;
            freeChunk(&function->chunk);
            FREE(OBJ_FUNCTION,object);
            break;
    }
}

void freeObjects(){
    Obj* object = vm.objects;
    while(object!=NULL){
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}
