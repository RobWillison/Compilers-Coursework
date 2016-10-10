#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>
#include "token.h"
#include "union.h"
#include "frame.h"

#define ANSWERVALUE 254
#define NOTHING 0

char *named(int t)
{
    static char b[100];
    if (isgraph(t) || t==' ') {
      sprintf(b, "%c", t);
      return b;
    }
    switch (t) {
      default: return "???";
    case IDENTIFIER:
      return "id";
    case CONSTANT:
      return "constant";
    case STRING_LITERAL:
      return "string";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case EXTERN:
      return "extern";
    case AUTO:
      return "auto";
    case INT:
      return "int";
    case VOID:
      return "void";
    case APPLY:
      return "apply";
    case LEAF:
      return "leaf";
    case IF:
      return "if";
    case ELSE:
      return "else";
    case WHILE:
      return "while";
    case CONTINUE:
      return "continue";
    case BREAK:
      return "break";
    case RETURN:
      return "return";
    }
}

void print_leaf(NODE *tree, int level)
{
    TOKEN *t = (TOKEN *)tree;
    int i;

    for (i=0; i<level; i++) putchar(' ');
    if (t->type == CONSTANT) printf("%d\n", t->value);
    else if (t->type == STRING_LITERAL) printf("\"%s\"\n", t->lexeme);
    else if (t) puts(t->lexeme);
}

void print_tree0(NODE *tree, int level)
{

    int i;
    if (tree==NULL) return;
    if (tree->type==LEAF) {
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar(' ');
      printf("%s\n", named(tree->type));
/*       if (tree->type=='~') { */
/*         for(i=0; i<level+2; i++) putchar(' '); */
/*         printf("%p\n", tree->left); */
/*       } */
/*       else */
        print_tree0(tree->left, level+2);
      print_tree0(tree->right, level+2);
    }
}

void print_tree(NODE *tree)
{
    print_tree0(tree, 0);
}

extern int yydebug;
extern NODE* yyparse(char* fileName);
extern NODE* ans;
extern TOKEN** init_symbtable(void);

//MY STUFF
extern UNION* intepret_body(NODE *tree, FRAME *enviroment);

UNION* new_result(int type, int value)
{
  UNION *ans = (UNION*)malloc(sizeof(UNION));
  ans->value = value;
  ans->type = type;
  return ans;
}

FRAME* new_frame()
{
  FRAME *ans = (FRAME*)malloc(sizeof(FRAME));
  ans->next = NULL;
  ans->value = NULL;
  return ans;
}

VARIABLE* new_variable(TOKEN *token, UNION *value)
{
  VARIABLE *ans = (VARIABLE*)malloc(sizeof(VARIABLE));
  ans->token = token;
  ans->value = value;
  ans->next = NULL;
  return ans;
}

void store_variable(FRAME *enviroment, TOKEN *token, UNION *value)
{
  VARIABLE *variable = new_variable(token, value);

  if(enviroment->value == NULL){
    enviroment->value = (VARIABLE*)&variable;
  } else {
    VARIABLE *current = (VARIABLE*)enviroment->value;
    while (current->next != NULL) {
      current = (VARIABLE*)current->next;
    }
    current->next = (VARIABLE*)&variable;
  }
}

UNION* lookup_variable(FRAME *enviroment, TOKEN *token)
{
  //Loop Through Frames
  while(enviroment->value != NULL)
  {
    VARIABLE *pointer = (VARIABLE*)enviroment->value;
    while(pointer != NULL)
    {
      if (pointer->token == token)
      {
        return pointer->value;
      }
      pointer = pointer->next;
    }
    enviroment = (FRAME*)enviroment->next;
  }
}


UNION* intepret_condition(NODE *tree, FRAME *enviroment)
{
  UNION *condition_result = intepret_body(tree->left, enviroment);

  //If its an if else
  if (tree->right->type == ELSE) {
    if(condition_result->value)
    {
      return intepret_body(tree->right->left, enviroment);
    } else {
      return intepret_body(tree->right->right, enviroment);
    }
  }

  if(condition_result->value)
  {
    return intepret_body(tree->right, enviroment);
  }

  //THIS IS A BODGE
  return new_result(NOTHING, 0);
}

UNION* intepret_math(NODE *tree, FRAME *enviroment)
{
  UNION *left_result = intepret_body(tree->left, enviroment);
  UNION *right_result = intepret_body(tree->right, enviroment);

  int left_value = left_result->value;
  int right_value = right_result->value;

  int result_value = 0;

  switch (tree->type) {

    case '<':
      result_value = left_value < right_value;
      break;

    case '>':
      result_value = left_value > right_value;
      break;

    case LE_OP:
      result_value = left_value <= right_value;
      break;

    case GE_OP:
      result_value = left_value >= right_value;
      break;

    case EQ_OP:
      result_value = left_value == right_value;
      break;

    case NE_OP:
      result_value = left_value != right_value;
      break;

    case '*':
      result_value = left_value * right_value;
      break;

    case '/':
      result_value = left_value / right_value;
      break;

    case '+':
      result_value = left_value + right_value;
      break;

    case '-':
      result_value = left_value - right_value;
      break;
  }

  UNION *result = new_result(INT, result_value);

  return result;
}

UNION* intepret_body(NODE *tree, FRAME *enviroment)
{
  printf("NEXT TREE\n");

  print_tree(tree);

  //Find the type of the node and do something appropriate
  UNION *result = new_result(NOTHING, 0);

  switch (tree->type) {
    case RETURN:
      result = intepret_body(tree->left, enviroment);
      result->type = ANSWERVALUE;

      return result;

    case IF:
      return intepret_condition(tree, enviroment);

    case '<':
    case '>':
    case '*':
    case '/':
    case '+':
    case '-':
    case LE_OP:
    case GE_OP:
    case EQ_OP:
    case NE_OP:
      return intepret_math(tree, enviroment);

    case '~':
      if (tree->left->left->type == INT) {
        TOKEN *token = (TOKEN*)tree->right->left;

        UNION *result = new_result(INT, token->value);

        store_variable(enviroment, token, result);
      }
      return;

    case LEAF:
      if(tree->left->type == CONSTANT) {
        TOKEN *t = (TOKEN *)tree->left;
        result->value = t->value;
        return result;
      }
      if(tree->left->type == IDENTIFIER) {
        TOKEN *t = (TOKEN *)tree->left;

        UNION *stored_value = (UNION*)lookup_variable(enviroment, t);

        //TODO
        result->value = stored_value->value;
        return result;
      }
  }

  if (tree->type = ';') {
    result = intepret_body(tree->left, enviroment);

    if (result->type == ANSWERVALUE) {
      return result;
    }


    result = intepret_body(tree->right, enviroment);
    if (result->type == ANSWERVALUE) {
      return result;
    }
  }
}

void interpret_definition(NODE *tree, FRAME *enviroment)
{
  //TODO
}

UNION* intepret(NODE *tree, FRAME *enviroment)
{
  //Get all definition
  switch(tree->type)
  {
    case '~':
      intepret(tree->left, enviroment);
      intepret(tree->right, enviroment);
    case 'D':
      interpret_definition(tree, enviroment);
  }


  return intepret_body(tree->right, enviroment);
}

int main(int argc, char** argv)
{
    char* fileName = "";
    NODE* tree;

    fileName = argv[1];

    if (argc>2 && strcmp(argv[2],"-d")==0) yydebug = 1;

    init_symbtable();
    printf("--C COMPILER\n");
    yyparse(fileName);
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);

    printf("Starting Interpretation\n");

    FRAME *enviroment = new_frame();
    UNION *result = (UNION*)intepret(tree, enviroment);

    printf("\n\RESULT: %d\n", result->value);

    return result->value;
}
