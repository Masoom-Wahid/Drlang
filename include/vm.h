#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
    // Bytecode , parsed by parser
    Chunk *chunk;
    // same as curr_token , this store the curr bytecode being executed
    // stands for Instruction Pointer
    uint8_t *ip;
    // the stack
    Value stack[STACK_MAX];
    // the top stack pointer
    Value *stackTop;
    // for strings and idents , better to keep them in a hashmap for fast lookup
    Table strings;
    // for heap alloacted objects
    Obj* objects;
} VM;


typedef enum{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();

InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif
