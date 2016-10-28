#ifndef __TOKEN_H
#define __TOKEN_H

#define TRUE 1
#define FALSE 0
#define TEST_MODE 0

typedef struct TOKEN
{
  int           type;
  char          *lexeme;
  int           value;
  struct TOKEN  *next;
} TOKEN;

extern TOKEN* new_token(int);

#endif

typedef struct node {
  int          type;
  struct node *left;
  struct node *right;
} NODE;

NODE* make_leaf(TOKEN*);
NODE* make_node(int, NODE*, NODE*);

typedef struct CLOSURE
{
  struct NODE   *ast;
  struct FRAME  *enviroment;
  struct NODE   *arguments;
} CLOSURE;

typedef struct UNION
{
  int           type;
  int           value;
  int           hasreturned;
  struct CLOSURE  *closure;
  struct UNION  *pointer;

} UNION;

extern UNION* new_result(int, int);

typedef struct FRAME
{
  struct VARIABLE  *value;
  struct MEMORY    *memory;
  struct FRAME  *next;
} FRAME;

extern FRAME* new_frame();

typedef struct VARIABLE
{
  struct TOKEN     *token;
  struct UNION     *value;
  struct VARIABLE  *next;
} VARIABLE;

extern VARIABLE* new_variable(TOKEN *, UNION *);

extern CLOSURE* new_closure(NODE *ast, FRAME *enviroment, NODE *arguments);
