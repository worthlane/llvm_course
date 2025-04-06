#include <stdio.h>

void funcStartLogger(char* func_name) {
  printf("[LOG] Start function '%s'\n", func_name);
}

/*void logInstruction(const char* opcode, int64_t* counter) {
  (*counter)++;
  printf("[LOG] Start instruction '%s', used %lld times\n", opcode, *counter);
}*/

void logInstruction(char* opcode) {
  printf("[LOG] Start instruction '%s'\n", opcode);
}
