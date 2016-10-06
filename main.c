#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>
#include "token.h"
#include "result.h"

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
extern void init_symbtable(void);

//MY STUFF
extern RESULT* intepret(NODE *tree);

RESULT* new_result(int value)
{
  RESULT *ans = (RESULT*)malloc(sizeof(RESULT));
  ans->value = value;
  return ans;
}

void store_variable(NODE *tree)
{
  //node->left is the leaf for the identifier;
  TOKEN *identifier = (TOKEN *) tree->left->left;
  char *name = identifier->lexeme;

  //node->right is the value;
  RESULT *value = intepret(tree->right);

  TOKEN *token = new_token(CONSTANT);
  token->lexeme = name;
  token->value = value->value;

  add_token(token);
}

int terminated = 0;

RESULT* intepret_condition(NODE *tree)
{
  RESULT *condition_result = intepret(tree->left);

  //If its an if else
  if (tree->right->type == ELSE) {
    if(condition_result->value)
    {
      return intepret(tree->right->left);
    } else {
      return intepret(tree->right->right);
    }
  }

  if(condition_result->value)
  {
    return intepret(tree->right);
  }

  //THIS IS A BODGE
  return new_result(0);
}

RESULT* intepret_math(NODE *tree)
{
  RESULT *left_result = intepret(tree->left);
  RESULT *right_result = intepret(tree->right);

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

  RESULT *result = new_result(result_value);

  return result;
}

RESULT* intepret(NODE *tree)
{

  printf("NEXT TREE\n");
  print_tree(tree);

  //Find the type of the node and do something appropriate
  RESULT *result = new_result(0);

  switch (tree->type) {

    case RETURN:
      result = intepret(tree->left);
      result->terminated = 1;
      return result;

    case IF:
      return intepret_condition(tree);

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
      return intepret_math(tree);

    case '~':
      //declaration
      //type is in left, value right
      store_variable(tree->right);
      return;

    case LEAF:
      if(tree->left->type == CONSTANT) {
        TOKEN *t = (TOKEN *)tree->left;
        result->value = t->value;
        return result;
      }
      if(tree->left->type == IDENTIFIER) {
        TOKEN *t = (TOKEN *)tree->left;
        char *identifier = t->lexeme;

        TOKEN *stored_value = (TOKEN *) (long) lookup_token(identifier);

        result->value = stored_value->value;
        return result;
      }
  }

  if (tree->type = ';') {
    result = intepret(tree->left);
    if (result->terminated) {
      return result;
    }

    result = intepret(tree->right);
    if (result->terminated) {
      return result;
    }
  }
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

    init_symbtable();
    RESULT *result = intepret(tree->right);

    printf("\n\nRESULT: %d\n", result->value);

    return 0;
}
