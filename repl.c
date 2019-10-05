#include <stdio.h>
#include <stdlib.h>

/* Windows compile target */
#ifdef _WIN32

#include <string.h>

static char input[2048];

/* Fake readline function */
char* readline(char* prompt)
{
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  ctrcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Linux and Mac complie target */
#else

#include <editline/readline.h>
#include <editline/history.h>

#endif

int main(int argc, char** argv)
{
  puts("Lispy version 0.0.1");
  puts("Press Ctrl+c to Exit\n");

  while(1)
  {

    char* input = readline("Lispy >>> ");

    add_history(input);

    printf("No you're a %s\n", input);

    free(input);
  }

  return 0;
}
