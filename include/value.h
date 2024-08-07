#ifndef drlang_value_h
#define drlang_value_h

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ,
} ValueType;

typedef struct {
    ValueType type;
    // so basically 'as ' can be either bool or number . seems like generics in C 
    // except if u fuck up , C goes levi mode and destroys everthing lol
    // and btw it will use the number of largest bits 
    // so since number is double it will use whatever double is using except even if 
    // the value is boolean
    union{
        bool boolean;
        double number;
        Obj *obj;
    } as;
} Value;

// checkers
#define IS_BOOL(value)((value).type == VAL_BOOL)
#define IS_NIL(value)((value).type == VAL_NIL)
#define IS_NUMBER(value)((value).type == VAL_NUMBER)
#define IS_OBJ(value)((value).type == VAL_OBJ)


// change drlang into native c
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)


// converters , change native c into dynamic drlang
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj*)object}})

typedef struct{
    int capacity;
    int count;
    Value *values;
} ValueArray;


void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);
bool valuesEqual(Value a, Value b);

#endif