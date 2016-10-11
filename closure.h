typedef struct CLOSURE
{
  struct NODE   *ast;
  struct FRAME  *enviroment;
  struct NODE   *arguments;
} CLOSURE;

extern CLOSURE* new_closure(NODE *ast, FRAME *enviroment, NODE *arguments);
