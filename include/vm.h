#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk *chunk;
    // same as curr_token , this store the curr bytecode being executed
    // stands for Instruction Pointer
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stackTop;
} VM;


typedef enum{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;;

void initVM();
void freeVM();

InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif
