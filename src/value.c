#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "value.h"
#include "object.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}


void writeValueArray(ValueArray* array, Value value){
    if(array->count< array->count+1){
        int old_cap = array->capacity;
        array->capacity = GROW_CAPACITY(old_cap);
        array->values = GROW_ARRAY(Value,array->values,old_cap,array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}


void printValue(Value value){
    switch(value.type){
        case VAL_NUMBER:  printf("%g",AS_NUMBER(value)); break;
        case VAL_NIL:     printf("hich"); break;
        case VAL_BOOL:    printf(AS_BOOL(value) ? "sahih" : "ghalat"); break;
        case VAL_OBJ:     printObject(value);
    }
   
}



bool valuesEqual(Value a , Value b){
    if(a.type!=b.type) return false;

    /*
    the reason memcmp() isnt used is because the struct Value
    uses 1 bytes for type and 8 for value 
    so by default C adds padding making it 16 
    comparing diffent sizes fields isnt the best thing to do 
    with memcmp()
    */

    switch(a.type){
        case VAL_BOOL : return AS_BOOL(a) == AS_BOOL(b); break;
        case VAL_NIL  : return true;break;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);break;
        case VAL_OBJ : return AS_OBJ(a) == AS_OBJ(b);break;
        default:return false;
    }
}