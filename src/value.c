#include <stdio.h>
#include "memory.h"
#include "value.h"

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
        case VAL_NIL:     printf("nil"); break;
        case VAL_BOOL:    printf(AS_BOOL(value) ? "true" : "false"); break;
    }
   
}