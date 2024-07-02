#include <stdio.h>
#include<stdlib.h>
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

typedef void (*ParseFn)();

typedef struct{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;


static void expression();
static void unary();
static void binary();
static void parserPrecedence();
static void grouping();
static void number();
// static ParseRule* getRule(TokenType type);


Parser parser;
Chunk *compilingChunk;

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL,PREC_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL,NULL,PREC_NONE},
    [TOKEN_LEFT_BRACE]= {NULL,NULL,PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL,NULL,PREC_NONE},
    [TOKEN_COMMA] = {NULL,NULL,PREC_NONE},
    [TOKEN_DOT] = {NULL,NULL,PREC_NONE},
    [TOKEN_MINUS] = {unary,binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL,binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL,NULL,PREC_NONE},
    [TOKEN_SLASH] = {NULL,binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL,binary, PREC_FACTOR},
    [TOKEN_BANG] = {NULL,NULL,PREC_NONE},
    [TOKEN_BANG_EQUAL]= {NULL,NULL,PREC_NONE},
    [TOKEN_EQUAL]= {NULL,NULL,PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL,NULL,PREC_NONE},
    [TOKEN_GREATER] = {NULL,NULL,PREC_NONE},
    [TOKEN_GREATER_EQUAL] = {NULL,NULL,PREC_NONE},
    [TOKEN_LESS] = {NULL,NULL,PREC_NONE},
    [TOKEN_LESS_EQUAL]= {NULL,NULL,PREC_NONE},
    [TOKEN_IDENTIFIER]= {NULL,NULL,PREC_NONE},
    [TOKEN_STRING]= {NULL,NULL,PREC_NONE},
    [TOKEN_NUMBER]= {number,NULL,PREC_NONE},
    [TOKEN_AND]= {NULL,NULL,PREC_NONE},
    [TOKEN_CLASS]= {NULL,NULL,PREC_NONE},
    [TOKEN_ELSE]= {NULL,NULL,PREC_NONE},
    [TOKEN_FALSE]= {NULL,NULL,PREC_NONE},
    [TOKEN_FOR]= {NULL,NULL,PREC_NONE},
    [TOKEN_FUN]= {NULL,NULL,PREC_NONE},
    [TOKEN_IF]= {NULL,NULL,PREC_NONE},
    [TOKEN_NIL]= {NULL,NULL,PREC_NONE},
    [TOKEN_OR]= {NULL,NULL,PREC_NONE},
    [TOKEN_PRINT]= {NULL,NULL,PREC_NONE},
    [TOKEN_RETURN]= {NULL,NULL,PREC_NONE},
    [TOKEN_SUPER]= {NULL,NULL,PREC_NONE},
    [TOKEN_THIS]= {NULL,NULL,PREC_NONE},
    [TOKEN_TRUE]= {NULL,NULL,PREC_NONE},
    [TOKEN_VAR]= {NULL,NULL,PREC_NONE},
    [TOKEN_WHILE]= {NULL,NULL,PREC_NONE},
    [TOKEN_ERROR]= {NULL,NULL,PREC_NONE},
    [TOKEN_EOF]= {NULL,NULL,PREC_NONE},
};

static void errorAt(Token *token,const char* msg){
    if (parser.panic_mode) return;
    parser.panic_mode = true;
    fprintf(stderr,"[line %d] Error",token->line);

    if(token->type == TOKEN_EOF){
        fprintf(stderr," at end");
    }else if(token->type == TOKEN_ERROR){

    }else{
        fprintf(stderr," at '.%*s'",token->length,token->start);
    }

    fprintf(stderr,":%s\n",msg);
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

static uint8_t makeConstant(Value value){
    int constant = addConstant(currentChunk(),value);
    if(constant > UINT8_MAX){
        errorAtCurrent("Too many constants in one chunk");
        return 0;
    }
    return (uint8_t)constant;
}

static void emitConstant(double value){
    emit_bytes(OP_CONSTANT,makeConstant(value));
}


static void parserPrecedence(Precedence precedence){
    advance();
    ParseFn prefixrule = getRule(parser.previous.type)->prefix;
    if(prefixrule == NULL){
        errorAtCurrent("Expected Expression");
        return;
    }

    prefixrule();

    while(precedence <= getRule(parser.current.type)->precedence){
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static void expression(){
    parserPrecedence(PREC_ASSIGNMENT);
}


static void binary(){
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parserPrecedence((Precedence)(rule->precedence + 1));
    switch(operatorType){
        case TOKEN_PLUS:    emit_byte(OP_ADD); break;
        case TOKEN_MINUS:   emit_byte(OP_SUBTRACT); break;
        case TOKEN_STAR:    emit_byte(OP_MULTIPLY); break;
        case TOKEN_SLASH:   emit_byte(OP_DIVIDE); break;
        default:            return; 
    }
}
static void number(){
    double value = strtod(parser.previous.start,NULL);
    emitConstant(value);
}


static void unary(){
    TokenType operatorType = parser.previous.type;

    parserPrecedence(PREC_UNARY);
    switch(operatorType){
        case TOKEN_MINUS : emit_byte(OP_NEGATE); break;
        default: return;
    }
}

static void grouping(){
    expression();
    consume(TOKEN_RIGHT_PAREN,"expected ')' after expression");
}


bool compile(const char* source,Chunk *chunk){
    initScanner(source);
    compilingChunk = chunk;
    parser.had_error = false;
    parser.panic_mode = false;
    advance();
    expression();
    consume(TOKEN_EOF,"Expect End Of Expression");
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