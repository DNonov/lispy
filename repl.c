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

/* Error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Lisp Value types */
enum { LVAL_NUM, LVAL_ERR };

/* Lisp Value data structure */
typedef struct
{
  int type;
  long num;
  int err;
} lval;

/* Number type lval */
lval lval_num(long x)
{
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* Error type lval */
lval lval_err(int x)
{
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* Print an lavl*/
void lval_print(lval v)
{
  switch (v.type)
  {
  case LVAL_NUM: printf("%li", v.num); break;
  case LVAL_ERR:
    if (v.err == LERR_DIV_ZERO) { printf("Error: Division by zero!"); }
    if (v.err == LERR_BAD_OP) { printf("Error: Invalid operator!"); }
    if (v.err == LERR_BAD_NUM) { printf("Error: Invalid number!"); }
    break;
  }
}

/* Print lval_print followed by new line */
void lval_println(lval v ) { lval_print(v); putchar('\n'); }

/* Use operator string to see which operation to perform */
lval eval_operator(lval x, char* operator, lval y)
{
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }


  if (strcmp(operator, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(operator, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(operator, "*") == 0) { return lval_num(x.num * y.num); }
  if (strcmp(operator, "/") == 0) {
    return y.num == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num);
  }
  return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t)
{
  if (strstr(t->tag, "number")) {
    /* Check if there is error in conversion */
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  /* The operator is always the second child */
  char* operator = t->children[1]->contents;

  lval x = eval(t->children[2]);

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
      lval result = eval(r.output);
      lval_println(result);
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
