#include "common.h"
#include "vm.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "compiler.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


VM vm;

static void resetStack(){
    vm.stackTop = vm.stack;
}


static Value peek(int);
static bool isFalsey(Value);
static void concatencate();
static void reverse_str();

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
// since for until the variable name is put onto hashtable
// it is in stack we read it from constant buffer and then push
// it into hashtable giving it a better lifetime
#define READ_STRING() AS_STRING(READ_CONSTANT())
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
                // pop();
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
            case OP_MODULO : {
                if (!IS_NUMBER(peek(0))  && IS_NUMBER(peek(1))){
                    runtimeError("Expected both operands to be number");
                    return INTERPRET_RUNTIME_ERROR;
                }

                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                double result = fmod(a,b);
                push(
                    NUMBER_VAL(result)
                );
                break;
            };
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

            case OP_REVERSE: {
                if(IS_STRING(peek(0))){
                    reverse_str();
                    break;
                }else if(IS_NUMBER(peek(0))){
                    push(
                        NUMBER_VAL(
                            -AS_NUMBER(pop())
                        )
                    );
                    break;
                }
                else{
                    runtimeError("Invalid Operand for '@' , The operands should be either String or Number");
                    return INTERPRET_RUNTIME_ERROR;
                }
            };
            case OP_PRINT:
                printValue(pop());
                printf("\n");
                break;
            case OP_POP: pop(); break;
            case OP_DEFINE_GLOBAL:
                ObjString* global_name = READ_STRING();
                tableSet(&vm.globals,global_name,peek(0));
                pop();
                break;
            
            case OP_GET_GLOBAL:
                ObjString* get_name = READ_STRING();
                Value value;
                if(!tableGet(&vm.globals,get_name,&value)){
                    runtimeError("undefined variable '%s' ",get_name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(value);
                break;
            case OP_SET_GLOBAL:
                ObjString* name = READ_STRING();
                /* 
                   tableSet will set the variable if it does not exist
                   so naturally implicit variable declaration is not accepted
                   say for ex:
                   baan x basha 10;
                   x basha 20;
                   y basha 30;
                   here y is not declared but we are reassigning it 
                   so if tableSet returns true it means that the variable did not exist
                   before , hence we delete it and return a rumtime error 
                   since the user is calling an undefined variable
                */
                if(tableSet(&vm.globals,name,peek(0))){
                    tableDelete(&vm.globals,name);
                    runtimeError("undefined variable '%s' .",name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                } 
                break;
            case OP_GET_LOCAL:
                uint8_t get_slot = READ_BYTE();
                push(vm.stack[get_slot]);
                break;
            case OP_SET_LOCAL:
                uint8_t set_slot = READ_BYTE();
                vm.stack[set_slot] = peek(0);
                break;
        }
    }

#undef READ_STRING
#undef BINARY_OP
#undef READ_CONSTANT
#undef READ_BYTE
}

void initVM(){
    resetStack();
    initTable(&vm.strings);
    initTable(&vm.globals);
    vm.objects = NULL;
}

void freeVM(){
    freeTable(&vm.strings);
    freeTable(&vm.globals);
    freeObjects();
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



static void reverse_str(){
    ObjString* str_obj = AS_STRING(pop());
    int length = str_obj->length;
    char* str = str_obj->chars;
    // allocate the data for the new str
    char* rev_str = ALLOCATE(char,length);

    // go backward and reverse the str
    for(int i = 0;i < length;i++){
        rev_str[i] = str[length-i-1];
    }

    // just add a trailing to make sure
    rev_str[length] = '\0';
    // printf("\t\t\t\t\tstr is : %s\n",str);
    // printf("\t\t\t\t\treversed_str is : %s\n",rev_str);

    // change it into an object
    ObjString* result = takeString(rev_str,length);
    push(OBJ_VAL(result));
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