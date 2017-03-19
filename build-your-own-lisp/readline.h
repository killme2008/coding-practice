#ifndef READLINE_H
#define READLINE_H

#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

/** If we are compiling on Mac OSX, only include readline.h */
#elif __APPLE__
#include <editline/readline.h>

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

#endif
