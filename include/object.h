#ifndef drlang_object_h
#define drlang_object_h

#include "value.h"
#include "chunk.h"

typedef enum{
    OBJ_FUNCTION,
    OBJ_STRING,
} ObjType;

struct Obj{
    ObjType type;
    struct Obj* next;
};

typedef struct{
    Obj obj;
    int arity;
    Chunk chunk;
    ObjString *name;
} ObjFunction;

struct ObjString{
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars,int length);
ObjFunction* newFunction();

void printObject(Value value);

static inline bool isObjType(Value value,ObjType type){
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#define IS_FUNCTION(value) isObjType(value,OBJ_FUNCTION)
#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) isObjType(value,OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))

#endif
