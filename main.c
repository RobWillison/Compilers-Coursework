#include <stdio.h>
#include <ctype.h>
#include "nodes.h"
#include "C.tab.h"
#include <string.h>

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



void store_variable(NODE *tree)
{
  //node->left is the leaf for the identifier;
  TOKEN *identifier = (TOKEN *) tree->left->left;
  char *name = identifier->lexeme;

  //node->right is the value;
  int value = intepret(tree->right);

  TOKEN *token = new_token(CONSTANT);
  token->lexeme = name;
  token->value = value;

  add_token(token);
}
//MY STUFF

int terminated = 0;

int intepret_if(NODE *tree)
{
  int condition_result = intepret(tree->left);

  //If its an if else
  if (tree->right->type == ELSE) {
    if(condition_result)
    {
      return intepret(tree->right->left);
    } else {
      return intepret(tree->right->right);
    }
  }

  if(condition_result)
  {
    return intepret(tree->right);
  }

}

int intepret(NODE *tree)
{
  printf("NEXT TREE\n");
  print_tree(tree);

  //Find the type of the node and do something appropriate
  switch (tree->type) {

    case RETURN:
      terminated = 1;
      return intepret(tree->left);

    case IF:
      return intepret_if(tree);

    case '<':
      return intepret(tree->left) < intepret(tree->right);

    case '>':
      return intepret(tree->left) > intepret(tree->right);

    case '*':
      return intepret(tree->left) * intepret(tree->right);

    case '/':
      return intepret(tree->left) / intepret(tree->right);

    case '+':
      return intepret(tree->left) + intepret(tree->right);

    case '-':
      return intepret(tree->left) - intepret(tree->right);

    case '~':
      //declaration
      //type is in left, value right
      store_variable(tree->right);
      return;

    case LEAF:
      if(tree->left->type == CONSTANT) {
        TOKEN *t = (TOKEN *)tree->left;
        return t->value;
      }
      if(tree->left->type == IDENTIFIER) {
        TOKEN *t = (TOKEN *)tree->left;
        char *identifier = t->lexeme;

        TOKEN *stored_value = (TOKEN *) (long) lookup_token(identifier);

        return stored_value->value;
      }
  }

  if (tree->type = ';') {
    int returned_value = intepret(tree->left);
    if (terminated) {
      return returned_value;
    }

    returned_value = intepret(tree->right);
    if (terminated) {
      return returned_value;
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
    int result = intepret(tree->right);

    printf("\n\nRESULT: %d\n", result);


    return 0;
}
