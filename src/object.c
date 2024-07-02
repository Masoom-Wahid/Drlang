#include <stdio.h>
#include <string.h>


#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"


// Created an Obj* which will be casted to that type
// so when Calling it with ObjString it will be converted Obj* to ObjString*
#define ALLOCATE_OBJ(type,objectype)\
    (type*)allocateObj(sizeof(type),objectype)

static Obj* allocateObj(size_t size,ObjType type){
    Obj* object = (Obj*)reallocate(NULL,0,size);
    object->type = type;
    return object;
}


static ObjString* allocateString(char* chars,int length){
    // changed that array of chars into an ObjString*
    ObjString* string = ALLOCATE_OBJ(ObjString,OBJ_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}


void printObject(Value value){
    switch(OBJ_TYPE(value)){
        case OBJ_STRING:
            printf("%s",AS_CSTRING(value));
            break;
    }
}



ObjString* copyString(const char* chars, int length){
    // alloacte for the chars so the chars are on heap
    char *heapchars = ALLOCATE(char,length+1);
    // after alloacting copy them
    memcpy(heapchars,chars,length);
    // place a terminator at the end of the string
    heapchars[length] = '\0';
    // turn the heap allocated string into and ObjString*
    return allocateString(heapchars,length);
}


ObjString* takeString(char* chars,int length){
    return allocateString(chars,length);
}