#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "definitions.h"
#include "C.tab.h"
#include <string.h>

void print_variable_list(VARIABLE *pointer)
{
  printf("LIST\n");
  while (pointer != 0)
  {

    TOKEN *variable = (TOKEN*)pointer->token;
    UNION *union_result = (UNION*)pointer->value;

    if (union_result->type == INT) {
      printf("%s, %d\n", variable->lexeme, union_result->value);
    } else {
      printf("%s, CLOSURE\n", variable->lexeme);
    }
    pointer = pointer->next;
  }
  printf("END\n");
}

void print_enviroment(FRAME *enviroment)
{
  while (enviroment != 0)
  {
    printf("NEXT ENV FRAME\n");
    print_variable_list(enviroment->value);
    printf("END FRAME\n");

    enviroment = enviroment->next;
  }
  printf("END\n");
}

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
