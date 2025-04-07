#include <stdio.h>
#include <stdlib.h>

static FILE* logf = NULL;
static const char* kOutputFile = "assets/dynamic.log";

void initLogFile();
void closeLogFile();

void initLogFile() {
  printf("initted %p\n", logf);

  logf = fopen(kOutputFile, "w");

  if (logf == NULL)
      logf = stderr;

  atexit(closeLogFile);
}

void closeLogFile() {
  fclose(logf);
}

void logInstruction(char* opcode_name, long int* counter, unsigned int id) {
  (*counter)++;
  fprintf(logf, "%u '%s' counter: %ld\n", id, opcode_name, *counter);
  fflush(logf);
}

