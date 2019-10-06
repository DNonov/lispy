#include <stdio.h>
#include <stdlib.h>
#include "parser/mpc.h"

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

/* Use operator string to see which operation to perform */
long eval_operator(long x, char* operator, long y)
{
  if (strcmp(operator, "+") == 0) { return x + y; }
  if (strcmp(operator, "-") == 0) { return x - y; }
  if (strcmp(operator, "*") == 0) { return x * y; }
  if (strcmp(operator, "/") == 0) { return x / y; }
  return 0;
}

long eval(mpc_ast_t* t)
{
  /* If tagged as number return it */
  if (strstr(t->tag, "number")) { return atoi(t->contents);}

  /* The operator is always the second child */
  char* operator = t->children[1]->contents;

  long x = eval(t->children[2]);

  /* Iterate over the rest of the children and combine */
  int i = 3;
  while (strstr(t->children[i]->tag, "expression")) {
    x = eval_operator(x, operator, eval(t->children[i]));
    i++;
  }

  return x;
}

int main(int argc, char** argv)
{
  /* Create some parsers */
  mpc_parser_t* Number     = mpc_new("number");
  mpc_parser_t* Operator   = mpc_new("operator");
  mpc_parser_t* Expression = mpc_new("expression");
  mpc_parser_t* Lispy      = mpc_new("lispy");

  /* Define them with the following language */
  mpca_lang(MPCA_LANG_DEFAULT,
  "                                                         \
    number     : /-?[0-9]+/;                                \
    operator   : '+' | '-' | '*' | '/';                     \
    expression : <number> | '('<operator> <expression>+')'; \
    lispy      : /^/ <operator> <expression>+ /$/;          \
  ",
  Number, Operator, Expression, Lispy);

  /* Initial repl info */
  puts("Lispy version 0.0.1");
  puts("Press Ctrl+c to Exit\n");

  /* Actual repl */
  while(1)
  {

    char* input = readline("Lispy >>> ");

    add_history(input);

    /* Attempt to Parse the user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r))
    {
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);
    }
    else
    {
      /* Otherwise Print the error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  /* Remove the paresers */
  mpc_cleanup(4, Number, Operator, Expression, Lispy);

  return 0;
}
