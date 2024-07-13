#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;
    const char *current;
    int line;
} Scanner;

Scanner scanner;


bool isAtEnd(){
    return *scanner.current == '\0';
}

static Token makeToken(TokenType type){
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;

}


static Token errorToken(const char* msg){
    Token token;
    token.type = TOKEN_ERROR;
    token.start = scanner.start;
    token.length = (int)strlen(msg);
    token.line = scanner.line;

    return token;
}
static char advance(){
    scanner.current++;
    return scanner.current[-1];
}


static char peek(){
    return *scanner.current;
}

static char peekNext(){
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static void skipWhitespace(){
    for(;;){
        char c= peek();
        switch(c){
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            case '/':
                if(peekNext() == '/'){
                    while(peek() != '\n' && !isAtEnd()) advance();
                }else{
                    return;
                }
                break;
                
            default:
                return;
        }
    }
}

static bool match(char expected){
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;
}

static bool isDigit(char c){
    return c >= '0' && c <= '9';
}

static Token string(){
    while(peek() != '"' && !isAtEnd()){
        if(peek() == '\n') scanner.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated String");

    advance();
    return makeToken(TOKEN_STRING);

}


static Token number(){
    while(isDigit(peek())) advance();
    if(peek() == '.' && isDigit(peekNext())){
        advance();
        while(isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static bool isAlpha(char c){
    return (c >= 'a' && c <= 'z') ||
            (c  >= 'A' && c <= 'Z') ||
            c == '_';
}

static TokenType checkKeyword(int start, int length,const char* rest, TokenType type) {
if (scanner.current - scanner.start == start + length &&
    memcmp(scanner.start + start, rest, length) == 0) 
    {
        return type;
    }

    return TOKEN_IDENTIFIER;
}
static TokenType identifer_type(){
    /*
    print -> parto
    var -> mthwl 
    nil -> hich
    = -> hast 
    == -> basha 
    > -> kalan_az
    < -> khord_az
    != -> nist
    ! -> ni
    and -> wa
    or -> ya


    + -> jama
    - -> manfi
    / -> taqsim
    * -> zarb
    

    */
    switch (scanner.start[0]) {
        case 'n' :
            if(scanner.current - scanner.start > 1){
                switch (scanner.start[1]){
                	case 'i' : return checkKeyword(1,2,"st",TOKEN_BANG);
			case 'a' : return checkKeyword(2,5,"basha",TOKEN_BANG_EQUAL); 
		};
            break;
        };
        //case 'q' : return checkKeyword(1,6,"aga_ni",TOKEN_ELSE);
 
	case 'g' : return checkKeyword(1,5,"halat",TOKEN_FALSE);
    case 'a': 
        if(scanner.current - scanner.start > 1){
            TokenType token = checkKeyword(1,5,"ga_ni",TOKEN_ELSE);
            if(token == TOKEN_IDENTIFIER) return checkKeyword(1,2,"ga",TOKEN_IF);
            return token;
        };
                // if(scanner.current - scanner.start > 1){
        //     TokenType token = checkKeyword(1,5,"ga_ni",TOKEN_ELSE);
        //     if (token == TOKEN_IDENTIFIER){
        //         return checkKeyword(1,2,"ga",TOKEN_IF);
        //     }
        //     return token;
        // };
        // return checkKeyword(1, 2, "ga", TOKEN_IF);
    case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        // case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'h': 
        if(scanner.current - scanner.start > 1){
            switch(scanner.start[1]){
                case 'i' : return checkKeyword(2, 2, "ch", TOKEN_NIL);	
                case 'a' : return checkKeyword(2,2,"st",TOKEN_EQUAL);
            }
            break;
        };
        // case 'y': return checkKeyword(1, 1, "a", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "arto", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        // sahi and super
    case 's': if (scanner.current - scanner.start > 1){
            switch(scanner.start[1]){
                case 'u' : return checkKeyword(2, 3, "per", TOKEN_SUPER);
                case 'a' : return checkKeyword(2,3,"hih",TOKEN_TRUE);
            }
            break;
        };
    case 'm' : 
        if(scanner.current - scanner.start > 1){
            switch(scanner.start[2]){
                case 's' : return checkKeyword(3,3,"awi",TOKEN_EQUAL_EQUAL); break;
                case 'n' : return checkKeyword(3,2,"fi",TOKEN_MINUS); break;
		        case 'h' : return checkKeyword(3,2,"wl",TOKEN_VAR); break;
            }
            break;
        };

        case 'b': 
        if(scanner.current - scanner.start > 1){
            switch(scanner.start[2]){
        	    //case 'a' : return checkKeyword(3,1,"n",TOKEN_VAR);
                //case 's' : return checkKeyword(3,2,"ha",TOKEN_EQUAL);
		        case 's' : return checkKeyword(3,2,"ha",TOKEN_EQUAL_EQUAL);
            }
            break;
        };
        // return checkKeyword(1, 3, "aan", TOKEN_VAR);
        // case 'w': 
        //     if (scanner.current - scanner.start > 1) {
        //         switch(scanner.start[1]){
        //             'h' : return checkKeyword(2, 3, "ile", TOKEN_WHILE);
        //             // 'a' : return TOKEN_AND;
        //         }
        //         break;
        // };
        case 'w':
        if (scanner.current - scanner.start > 1) {
            switch (scanner.start[1]) {
                // case 'h': return checkKeyword(2, 3, "ile", TOKEN_WHILE);
                case 'a' : return checkKeyword(1,1,"a",TOKEN_AND);
            }
            break;
        };
        case 'f':
        if (scanner.current - scanner.start > 1) {
            switch (scanner.start[1]) {
                // case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
            }
            break;
        };
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'q' : return checkKeyword(2,3,"sim",TOKEN_SLASH);
                    case 'a' : return checkKeyword(1,1,"a",TOKEN_FOR);
                    case 'w' : return checkKeyword(2,7,"akht_ki",TOKEN_WHILE);
		};
            };
            break;
        
        case 'k':
            if (scanner.current - scanner.start > 1){
                switch (scanner.start[1]){
                    case 'a' :  return checkKeyword(2,6,"lan_az",TOKEN_GREATER);
                    case 'h' : return checkKeyword(2,6,"ord_az",TOKEN_LESS);
                }
            };
            break;

        case 'j' : return checkKeyword(1,3,"ama",TOKEN_PLUS);
        case 'z' : return checkKeyword(1,3,"arb",TOKEN_STAR);
        case 'y' : return checkKeyword(1,1,"a",TOKEN_OR);

    }

    return TOKEN_IDENTIFIER;
}

static Token identifer(){
    while ( isAlpha(peek()) || isDigit(peek())  ){
        advance();
    }

    return makeToken(identifer_type());
}

Token scanToken(){
    skipWhitespace();
    scanner.start = scanner.current;
    if(isAtEnd()) return makeToken(TOKEN_EOF);
    char c = advance();

    if(isAlpha(c)) return identifer();
    if (isDigit(c)) return number();
    switch(c){
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        // case '-': return makeToken(TOKEN_MINUS);
        // case '+': return makeToken(TOKEN_PLUS);
        // case '/': return makeToken(TOKEN_SLASH);
        // case '*': return makeToken(TOKEN_STAR);
        case '@': return makeToken(TOKEN_REVERSE);
        case '%': return makeToken(TOKEN_MODULO);
        case '!' : return makeToken(TOKEN_BANG);
        // case '-' : return makeToken(TOKEN_COMMENT);
        // case '!': return makeToken(
        //     match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG
        // );
        // case '=': return makeToken(
        //     match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
        // );
        // case '<': return makeToken(
        //     match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS
        // );
        // case '>': return makeToken(
        //     match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
        // );

        case '"' : return string();
    }

    return errorToken("Unexpected charectar");
}


void initScanner(const char* source){
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}
