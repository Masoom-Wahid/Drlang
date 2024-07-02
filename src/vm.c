#include "common.h"
#include "vm.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "compiler.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


VM vm;

static void resetStack(){
    vm.stackTop = vm.stack;
}


static Value peek(int);
static bool isFalsey(Value);
static void concatencate();

static void runtimeError(const char* format,...){
    va_list args;
    va_start(args,format);
    vfprintf(stderr,format,args);
    va_end(args);
    fputs("\n",stderr);
    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr,"[line %d] in script \n",line);
    resetStack();
}

static InterpretResult run(){
#define READ_BYTE() (*vm.ip++)
// read_byte() returns the next byte , which in turn it is where the index is stored so
// indexing the constants.value gives u operand of the op_constant
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

/* 
    if both operands which should be at the top of stack 
    smth like this
    1 2 OP_ADD
    are not number then we should give a runtime error 
    otherwise just get their values 
    and then push them back
*/
#define BINARY_OP(valueType, op) \
do { \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
    } \
    double b = AS_NUMBER(pop()); \
    double a = AS_NUMBER(pop()); \
    push(valueType(a op b)); \
} while (false)


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
                // printf("\n");
                return INTERPRET_OK;
            case OP_CONSTANT:
                Value constant = READ_CONSTANT();
                push(constant);
                // printValue(constant);
                // printf("\n");
                break;
            case OP_NEGATE:
                if(!IS_NUMBER(peek(0))){
                    runtimeError("Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }

                /*
                just for clarity
                this pushes pops from the the stack
                gets the numbers and then negates it 
                and then creates a number from it using
                NUMBER_VAL and push it back to stack again
                */
                push(
                    NUMBER_VAL(
                        -AS_NUMBER(pop())
                        )
                );
                break;
            case OP_ADD:  {
                if(IS_STRING(peek(0))  && IS_STRING(peek(1))){
                    concatencate();
                    break;
                }else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
                    double a = AS_NUMBER(pop());
                    double b = AS_NUMBER(pop());
                    push(NUMBER_VAL(a+b));
                    break;
                }else{
                    runtimeError("Invalid datatype for operator +");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // BINARY_OP(NUMBER_VAL,+); break
            };
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL,-); break;
            case OP_MULTIPLY : BINARY_OP(NUMBER_VAL,*); break;
            case OP_DIVIDE: BINARY_OP(NUMBER_VAL,/); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_NIL: push(NIL_VAL); break;
            case OP_NOT: push(BOOL_VAL(isFalsey(pop()))); break;
            case OP_EQUAL:
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a,b)));
                break;
            
            case OP_GREATER: BINARY_OP(BOOL_VAL,>); break;
            case OP_LESS:   BINARY_OP(BOOL_VAL,<); break;

            // case OP_GREATER:
            //     Value b = 
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

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}


static bool isFalsey(Value value){
    /*
    nil and false are falsey and every other value
    behaves like true .
    */
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatencate(){
    ObjString* bstr = AS_STRING(pop());
    ObjString* astr = AS_STRING(pop());

    int length = astr->length + bstr->length;
    // printf("length was %d\n",length);
    char* chars = ALLOCATE(char,length+1);

    // copy astr = from start of char until lenght of astr
    memcpy(chars,astr->chars,astr->length);

    // copy bstr from where astr finished and then start bstr until end
    memcpy(chars+astr->length,bstr->chars,bstr->length);
    
    
    chars[length] = '\0';

    // printf("%s\n",chars);

    ObjString* result  = takeString(chars,length);

    push(OBJ_VAL(result));
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