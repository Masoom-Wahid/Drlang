#include <stdio.h>
#include <string.h>


#include "chunk.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"


// Created an Obj* which will be casted to that type
// so when Calling it with ObjString it will be converted Obj* to ObjString*
#define ALLOCATE_OBJ(type,objectype)\
    (type*)allocateObj(sizeof(type),objectype)

static Obj* allocateObj(size_t size,ObjType type){
    Obj* object = (Obj*)reallocate(NULL,0,size);
    object->type = type;
    object->next = vm.objects;
    vm.objects = object;
    return object;
}


ObjFunction* newFunction(){
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

static ObjString* allocateString(char* chars,int length,uint32_t hash){
    // changed that array of chars into an ObjString*
    ObjString* string = ALLOCATE_OBJ(ObjString,OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    tableSet(&vm.strings,string,NIL_VAL);
    return string;
}

static uint32_t hashString(const char* key,int length){
    // “FNV-1a” hashing algorithm
    uint32_t hash = 2166136261u;
    for(int i = 0;i < length;i++){
        hash ^= key[i];
        hash *= 16777619;
    }
    return hash;
}


static void printFunction(ObjFunction* function){
    if(function->name == NULL){
        printf("<script>");
        return;
    }
    printf("<fn %s>",function->name->chars);
}

void printObject(Value value){
    switch(OBJ_TYPE(value)){
        case OBJ_STRING:
            printf("%s",AS_CSTRING(value));
            break;
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
    }
}



ObjString* copyString(const char* chars, int length){
    // hash the string for the hashtable
    uint32_t hash = hashString(chars,length);
    // if the curr string already exists then return that instead of again copying it
    ObjString* interned = tableFindString(&vm.strings,chars,length,hash);
    if(interned != NULL) return interned;

    // alloacte for the chars so the chars are on heap
    char *heapchars = ALLOCATE(char,length+1);
    // after alloacting copy them
    memcpy(heapchars,chars,length);
    // place a terminator at the end of the string
    heapchars[length] = '\0';
    // turn the heap allocated string into and ObjString*
    return allocateString(heapchars,length,hash);
}


ObjString* takeString(char* chars,int length){
    uint32_t hash = hashString(chars,length);
    /*
    if the curr string exists than the curr pointer is useless
    instead we free the curr pointer and give the one we have in
    our hash table
    */
    ObjString* interned = tableFindString(&vm.strings,chars,length,hash);
    if(interned != NULL) {
        FREE_ARRAY(char,chars,length+1);
        return interned;
    };
    // else we alloacte and give the ownership
    return allocateString(chars,length,hash);
}
