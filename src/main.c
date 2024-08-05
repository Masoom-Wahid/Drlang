#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void repl() {
  // func 1 byte
  char line[1024];
  for (;;) {
    printf("> ");
    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }
    // printf("%s\n",line);
    InterpretResult result = interpret(line);
    if (result == INTERPRET_COMPILE_ERROR)
      exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
      exit(70);
  }
}

static char *readFile(const char *path) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  };
  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(file_size + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }
  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);

  // if for some reason we can't read all bytes properly
  if (bytes_read < file_size) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(file);

  return buffer;
}

static void runFile(const char *path) {
  char *source = readFile(path);
  if (source == NULL) {
    fprintf(stderr, "Invalid file\n");
    exit(74);
  }
  InterpretResult result = interpret(source);
  free(source);
  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

int main(int argc, const char *argv[]) {
  initVM();
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: drlang [path]\n");
    exit(64);
  }

  freeVM();
  return 0;
}

// int main(int argc,const char* argv[]){
//     printf("-- Welcome to Clox-- \n");
//     initVM();
//     Chunk chunk;
//     initChunk(&chunk);
//     // int constant = addConstant(&chunk,1.2);
//     // writeChunk(&chunk,OP_CONSTANT,123);
//     // writeChunk(&chunk,constant,123);
//     // writeChunk(&chunk,OP_NEGATE,123);
//     // int another_constant = addConstant(&chunk,2);
//     // writeChunk(&chunk,OP_CONSTANT,123);
//     // writeChunk(&chunk,another_constant,123);
//     // writeChunk(&chunk,OP_ADD,123);
//     // writeChunk(&chunk,OP_RETURN,123);
//     // disassembleChunk(&chunk,"test chunk");

//     // int constant = addConstant(&chunk, 1.2);
//     // writeChunk(&chunk, OP_CONSTANT, 123);
//     // writeChunk(&chunk, constant, 123);
//     // constant = addConstant(&chunk, 3.4);
//     // writeChunk(&chunk, OP_CONSTANT, 123);
//     // writeChunk(&chunk, constant, 123);
//     // writeChunk(&chunk, OP_ADD, 123);
//     // constant = addConstant(&chunk, 5.6);
//     // writeChunk(&chunk, OP_CONSTANT, 123);
//     // writeChunk(&chunk, constant, 123);
//     // writeChunk(&chunk, OP_DIVIDE, 123);
//     // writeChunk(&chunk, OP_NEGATE, 123);
//     // writeChunk(&chunk, OP_RETURN, 123);

//     int cons = addConstant(&chunk,1);
//     writeChunk(&chunk,OP_CONSTANT,123);
//     writeChunk(&chunk,cons,123);

//     cons = addConstant(&chunk,2);
//     writeChunk(&chunk,OP_CONSTANT,123);
//     writeChunk(&chunk,cons,123);

//     writeChunk(&chunk,OP_MULTIPLY,123);

//     cons = addConstant(&chunk,3);
//     writeChunk(&chunk,OP_CONSTANT,123);
//     writeChunk(&chunk,cons,123);

//     writeChunk(&chunk,OP_ADD,123);
//     interpret(&chunk);
//     freeVM();
//     freeChunk(&chunk);
//     return 0;
// }
