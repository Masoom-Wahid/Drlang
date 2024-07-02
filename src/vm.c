#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include<stdio.h>


VM vm;

static void resetStack(){
    vm.stackTop = vm.stack;
}


static InterpretResult run(){
#define READ_BYTE() (*vm.ip++)
// read_byte() returns the next byte , which in turn it is where the index is stored so
// indexing the constants.value gives u operand of the op_constant
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)\
    do{ \
        double b = pop();\
        double a = pop();\
        push(a op b);\
    }while (false)


    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            printf("        STACK REPORT     ");
            for (Value *slot = vm.stack;slot < vm.stackTop;slot++){
                printf("[");
                printValue(*slot);
                printf("]");
            }
            printf("\n");
            // curr pointer minus the top poitner should give us the offset
            disassembleInstruction(vm.chunk,(int)(vm.ip-vm.chunk->code));
        #endif

        uint8_t instruction;
        switch(instruction = READ_BYTE()){
            case OP_RETURN:
                pop();
                // printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            case OP_CONSTANT:
                Value constant = READ_CONSTANT();
                push(constant);
                // printValue(constant);
                printf("\n");
                break;
            case OP_NEGATE:
                push(-pop()); break;
            case OP_ADD:  BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY : BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
        }
    }


#undef BINARY_OP
#undef READ_CONSTANT
#undef READ_BYTE
}

void initVM(){
    resetStack();
}

void freeVM(){

}


void push(Value value){
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop(){
    vm.stackTop--;
    return *vm.stackTop;
}


InterpretResult interpret(const char* source){
    Chunk chunk;
    initChunk(&chunk);
    if (!compile(source,&chunk)){
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = chunk.code;
    InterpretResult result = run();
    freeChunk(&chunk);
    return result;
    // vm.chunk = chunk;
    // vm.ip = vm.chunk->code;
    // return run();
}