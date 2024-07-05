#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"


#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct{
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum{
    PREC_NONE, // NONE 
    PREC_ASSIGNMENT,// =
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREC_FACTOR,  // * /
    PREC_UNARY, // ! -
    PREC_CALL, // . ()
    PREC_PRIMARY // STR | NUMBER | EXPR | ANY
}Precedence;

typedef void (*ParseFn)(bool can_assign);

typedef struct{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct{
    Token name;
    int depth;
} Local;

typedef struct{
    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} Compiler;



static void expression();
static void parserPrecedence();
static void unary(bool can_assign);
static void binary(bool can_assign);
static void grouping(bool can_assign);
static void number(bool can_assign);
static void literal(bool can_assign);
static void string(bool can_assign);
static void statement();
static void declaration();
static void variable(bool can_assign);
// static ParseRule* getRule(TokenType type);


Parser parser;
Chunk *compilingChunk;
Compiler* current = NULL;


ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL,PREC_NONE},
    [TOKEN_REVERSE] = {unary,NULL,PREC_TERM},
    [TOKEN_RIGHT_PAREN] = {NULL,NULL,PREC_NONE},
    [TOKEN_LEFT_BRACE]= {NULL,NULL,PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL,NULL,PREC_NONE},
    [TOKEN_COMMA] = {NULL,NULL,PREC_NONE},
    [TOKEN_DOT] = {NULL,NULL,PREC_NONE},
    [TOKEN_MINUS] = {unary,binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL,binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL,NULL,PREC_NONE},
    [TOKEN_SLASH] = {NULL,binary, PREC_FACTOR},
    [TOKEN_MODULO] = {NULL,binary,PREC_FACTOR},
    [TOKEN_STAR] = {NULL,binary, PREC_FACTOR},
    [TOKEN_BANG] = {unary,NULL,PREC_NONE},
    [TOKEN_BANG_EQUAL]= {NULL,binary,PREC_EQUALITY},
    [TOKEN_EQUAL]= {NULL,NULL,PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL,binary,PREC_EQUALITY},
    [TOKEN_GREATER] = {NULL,binary,PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL,binary,PREC_COMPARISON},
    [TOKEN_LESS] = {NULL,binary,PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]= {NULL,binary,PREC_COMPARISON},
    [TOKEN_IDENTIFIER]= {variable,NULL,PREC_NONE},
    [TOKEN_STRING]= {string,NULL,PREC_NONE},
    [TOKEN_NUMBER]= {number,NULL,PREC_NONE},
    [TOKEN_AND]= {NULL,NULL,PREC_NONE},
    [TOKEN_CLASS]= {NULL,NULL,PREC_NONE},
    [TOKEN_ELSE]= {NULL,NULL,PREC_NONE},
    [TOKEN_FALSE]= {literal,NULL,PREC_NONE},
    [TOKEN_FOR]= {NULL,NULL,PREC_NONE},
    [TOKEN_FUN]= {NULL,NULL,PREC_NONE},
    [TOKEN_IF]= {NULL,NULL,PREC_NONE},
    [TOKEN_NIL]= {literal,NULL,PREC_NONE},
    [TOKEN_OR]= {NULL,NULL,PREC_NONE},
    [TOKEN_PRINT]= {NULL,NULL,PREC_NONE},
    [TOKEN_RETURN]= {NULL,NULL,PREC_NONE},
    [TOKEN_SUPER]= {NULL,NULL,PREC_NONE},
    [TOKEN_THIS]= {NULL,NULL,PREC_NONE},
    [TOKEN_TRUE]= {literal,NULL,PREC_NONE},
    [TOKEN_VAR]= {NULL,NULL,PREC_NONE},
    [TOKEN_WHILE]= {NULL,NULL,PREC_NONE},
    [TOKEN_ERROR]= {NULL,NULL,PREC_NONE},
    [TOKEN_EOF]= {NULL,NULL,PREC_NONE},
};

static void errorAt(Token *token,const char* msg){
    if (parser.panic_mode) return;
    parser.panic_mode = true;
    fprintf(stderr,"[line %d]\nError ",token->line);

    if(token->type == TOKEN_EOF){
        fprintf(stderr," at end");
    }else if(token->type == TOKEN_ERROR){

    }else{
        fprintf(stderr,"at:\n-----------------\n\n%*s\n-----------------\n",token->length,token->start);
    }

    fprintf(stderr,"\n%s\n",msg);
    parser.had_error = true;
}


static ParseRule* getRule(TokenType type){
    return &rules[type];
}

static void errorAtCurrent(const char* msg){
    errorAt(&parser.current,msg);
}

static void advance(){
    parser.previous = parser.current;
    for(;;){
        parser.current  = scanToken();
        if(parser.current.type != TOKEN_ERROR) break;
        errorAtCurrent(parser.current.start);
    }
}



static void consume(TokenType type, const char* msg){
    if(parser.current.type == type){
        advance();
        return;
    }

    errorAtCurrent(msg);
}

static bool check(TokenType type){
    return type == parser.current.type;
}

static bool match(TokenType type){
    if(!check(type)) return false;
    advance();
    return true;
}



static Chunk* currentChunk(){
    return compilingChunk;
}

static void emit_byte(uint8_t byte){
    writeChunk(currentChunk(),byte,parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
}



static void emit_return(){
    emit_byte(OP_RETURN);
}


static void end_compiler(){
    emit_return();
    #ifdef DEBUG_PRINT_CODE
    if(!parser.had_error){
        disassembleChunk(currentChunk(),"   CODE REPORT    ");
    }
    #endif
}

static void beginScope(){
    current->scopeDepth++;
}

static void endScope(){
    // if a scope is finished we go through until we hit the lower scope
    // and submit emit_byte so the vm stack pops the local variables
    current->scopeDepth--;
    while(current->localCount > 0 && current->locals[current->localCount -1].depth >current->scopeDepth){
        emit_byte(OP_POP);
        current->localCount--;
    }
}

static uint8_t makeConstant(Value value){
    int constant = addConstant(currentChunk(),value);
    if(constant > UINT8_MAX){
        errorAtCurrent("Too many constants in one chunk");
        return 0;
    }
    return (uint8_t)constant;
}

static void emitConstant(Value value){
    emit_bytes(OP_CONSTANT,makeConstant(value));
}

static void initCompiler(Compiler* compiler){
    compiler->localCount=0;
    compiler->scopeDepth=0;
    current = compiler;
}


static void parserPrecedence(Precedence precedence){
    advance();
    ParseFn prefixrule = getRule(parser.previous.type)->prefix;
    if(prefixrule == NULL){
        errorAtCurrent("Expected Expression");
        return;
    }

    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefixrule(can_assign);

    while(precedence <= getRule(parser.current.type)->precedence){
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(can_assign);
    }

    // if it should be assignment and got messed up somewhere for example
    // baan x basha 10;
    // baan y basha 20;
    // x * y = 20+20;
    // this isnt valid and blocks the way for paser precedence
    // since the TOKEN_EQUAL hasnt been eaten up
    // so we give a runtime error
    if (can_assign && match(TOKEN_EQUAL)){
        errorAtCurrent("Invalid assignment target");
    }
}


static void markeInitilized(){
    current->locals[current->localCount-1].depth  = current->scopeDepth;
}

static uint8_t identifierConstant(Token *name){
    return makeConstant(
        OBJ_VAL(
            copyString(
                name->start,name->length
            )
        )
    );
}

static bool idents_equal(Token *a,Token *b){
    if(a->length != b->length) return false;
    return memcmp(a->start,b->start,a->length) == 0;
}


static int resolveLocal(Compiler* compiler,Token* name){
    // walk the array backwards until u find that varibale
    // this also mean that a scope lower can access a higher scope variable

    for(int i = compiler->localCount - 1;i >= 0;i--){
        Local *local = &compiler->locals[i];
        if(idents_equal(name,&local->name)){
            if(local->depth == -1){
                errorAtCurrent("Can't read local variable in it's own initializer");
            }
            return i;
        }
    }

    return -1;
}


static void addLocal(Token name){
    /*
    create a new local variable
    the data of the token is stored in token->current and token->length
    which points to the original source code
    so the local variable is literally the original source code (kinda)
    we dont alloc for it . (for good reason so)
    */


    // just to make sure we dont overflow the array (this aint rust)
    if(current->localCount  == UINT8_COUNT){
        errorAtCurrent("Too many local variables");
        return;
    }

    // get the new local point
    Local* local = &current->locals[current->localCount++];
    // put the token there
    local->name = name;
    // set the depth uninitialized
    local->depth = -1;
    // update the depth
    local->depth = current->scopeDepth;
}


static void declareVariable(){
    if(current->scopeDepth == 0) return;
    Token* name = &parser.previous;
    // check all the variables in the curr scope 
    // if their names equals then it is an error
    // else we break and add the variable
    for(int i = current->localCount - 1;i >= 0;i--){
        Local *local = &current->locals[i];
        if(local->depth != -1 && local->depth < current->scopeDepth){
            break;
        }

        if(idents_equal(name, &local->name)){
            errorAtCurrent("a variable with this name already exists");
        }
    }
    addLocal(*name);
}

static void defineVariable(uint8_t global){
    if(current->scopeDepth > 0){
        markeInitilized();
        return;
    }
    emit_bytes(OP_DEFINE_GLOBAL,global);
}

static uint8_t parseVariable(const char* msg){
    // just eats the ident token and if not gives the error
    // if it succesfully does the job it will create the variable identifierConstant()
    consume(TOKEN_IDENTIFIER,msg);
    // this should handle the local and global case
    declareVariable();
    // if it is a local variable than we dont store it in constant table hence returning 0
    if(current->scopeDepth > 0) return 0;
    return identifierConstant(&parser.previous);
}

static void expression(){
    parserPrecedence(PREC_ASSIGNMENT);
}

static void block(){
    while(!check(TOKEN_RIGHT_BRACE) && check(TOKEN_EOF)){
        declaration();
    }
    consume(TOKEN_RIGHT_BRACE,"expected '}' at the end of block .");
}

static void printStatement(){
    expression();
    consume(TOKEN_SEMICOLON,"Expect ';' at the end of statement");
    emit_byte(OP_PRINT);

}

static void synchronize(){
    parser.panic_mode = false;
    while(parser.current.type != TOKEN_EOF){
        if(parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type){
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                ;
        }

        advance();
    }
}

static void expressionStatement(){
    expression();
    consume(TOKEN_SEMICOLON,"Expect ; after expression");
    emit_byte(OP_POP);
}



static void varDeclaration(){
    uint8_t global = parseVariable("Expected variable name");
    if(match(TOKEN_EQUAL)){
        expression();
    }else{
        emit_byte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON,"Expected ';' at the end of statement");
    defineVariable(global);
}

static void statement(){
    if(match(TOKEN_PRINT)){
        printStatement();
    }else{
        expressionStatement();
    }
}

static void declaration(){
    if(match(TOKEN_VAR)){
        varDeclaration();
    }else if(match(TOKEN_LEFT_BRACE)){
        beginScope();
        block();
        endScope();
    }else{
        statement();
    }
    if (parser.panic_mode) synchronize();
}



static void literal(bool can_assign){
    switch(parser.previous.type){
        case TOKEN_FALSE: emit_byte(OP_FALSE); break;
        case TOKEN_TRUE : emit_byte(OP_TRUE); break;
        case TOKEN_NIL:   emit_byte(OP_NIL); break;
        default: return;
    }
}


static void string(bool can_assign){
    // +1 and -2 is to emit "" from the string and just have the string itself
    emitConstant(OBJ_VAL(
        copyString(
                parser.previous.start + 1,
                parser.previous.length -2
            )
        ));
}

static void namedVariable(Token name,bool can_assign){
    uint8_t getOp,setOp;
    int arg = resolveLocal(current,&name);
    if(arg != -1){
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }else{
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if( can_assign && match(TOKEN_EQUAL)){
        expression();
        emit_bytes(setOp,(uint8_t)arg);
    }else{
        emit_bytes(getOp,(uint8_t)arg);
    };
}

static void variable(bool can_assign){
    namedVariable(parser.previous,can_assign);
}

static void binary(bool can_assign){
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parserPrecedence((Precedence)(rule->precedence + 1));
    switch(operatorType){
        // arithmethic tokens
        case TOKEN_MODULO:          emit_byte(OP_MODULO); break;
        case TOKEN_PLUS:            emit_byte(OP_ADD); break;
        case TOKEN_MINUS:           emit_byte(OP_SUBTRACT); break;
        case TOKEN_STAR:            emit_byte(OP_MULTIPLY); break;
        case TOKEN_SLASH:           emit_byte(OP_DIVIDE); break;
        // Equaltiy tokens
        case TOKEN_BANG_EQUAL:      emit_bytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:     emit_byte(OP_EQUAL); break;
        case TOKEN_GREATER:         emit_byte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL:   emit_bytes(OP_LESS,OP_NOT); break;
        case TOKEN_LESS:            emit_byte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:      emit_bytes(OP_GREATER,OP_NOT); break;
        default:            return; 
    }
}
static void number(bool can_assign){
    double value = strtod(parser.previous.start,NULL);
    emitConstant(NUMBER_VAL(value));
}


static void unary(bool can_assign){
    TokenType operatorType = parser.previous.type;
    parserPrecedence(PREC_UNARY);
    switch(operatorType){
        case TOKEN_MINUS : emit_byte(OP_NEGATE); break;
        case TOKEN_BANG : emit_byte(OP_NOT); break;
        case TOKEN_REVERSE: emit_byte(OP_REVERSE); break;
        default: return;
    }
}

static void grouping(bool can_assign){
    expression();
    consume(TOKEN_RIGHT_PAREN,"expected ')' after expression");
}



bool compile(const char* source,Chunk *chunk){
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler);
    compilingChunk = chunk;
    parser.had_error = false;
    parser.panic_mode = false;
    advance();
    while(!match(TOKEN_EOF)){
        declaration();
    }
    // expression();
    // consume(TOKEN_EOF,"Expect End Of Expression");
    end_compiler();
    return !parser.had_error;
    // int line  = -1;
    // for(;;){
    //     Token token = scanToken();
    //     if(token.line != line){
    //         printf("\nline: [%4d]\n",token.line);
    //         line = token.line;
    //     }// }else{
    //     //     printf("    | ");
    //     // }
    //     // printf("line is : [%d]",token.line);
    //     printf("type:[%2d] token: => %.*s\n",token.type,token.length,token.start);
    //     if (token.type == TOKEN_EOF) break;
    // }
}