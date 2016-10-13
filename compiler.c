#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "debug.h"
#include "TACstruct.h"

int current_reg = 0;

#define LOCTOKEN 5
#define LOCREG 4

extern TAC *compile(NODE *tree);

void add_TAC_to_list(TAC *front, TAC *tail)
{
  while (front->next != 0) front = front->next;
  front->next = tail;
}

char *get_location(LOCATION *loc)
{
  if (loc->type == LOCREG)
  {
    char *string = malloc(sizeof(char) * 5);
    sprintf(string, "r%d", loc->reg);
    return string;
  } else {
    TOKEN *t = (TOKEN*)loc->token;
    if (t->type == CONSTANT) {
      char *result = malloc(sizeof(char) * 3);
      sprintf(result, "%d", t->value);
      return result;
    } else {
      return t->lexeme;
    }
  }
}

void print_tac(TAC *tac_code)
{
  if (tac_code == 0) return;

  print_tac(tac_code->next);

  if (tac_code->operation == 'S') {
    LOCATION *destination = tac_code->destination;
    LOCATION *operand_one = tac_code->operand_one;

    printf("%s := %s\n", get_location(destination), get_location(operand_one));
  } else {
    LOCATION *destination = tac_code->destination;
    LOCATION *operand_one = tac_code->operand_one;
    LOCATION *operand_two = tac_code->operand_two;

    printf("%s := %s %s %s\n", get_location(destination), get_location(operand_one), named(tac_code->operation), get_location(operand_two));
  }



}

TAC *new_tac(int destination)
{
  TAC *tac_struct = (TAC*)malloc(sizeof(TAC));
  tac_struct->destination = destination;
  tac_struct->next = 0;
  return tac_struct;
}

LOCATION *new_location(int type)
{
  LOCATION *loc = (LOCATION*)malloc(sizeof(LOCATION));
  loc->type = type;
  return loc;
}

LOCATION *next_reg()
{
  current_reg += 1;
  LOCATION *loc = new_location(LOCREG);
  loc->reg = current_reg;
  return loc;
}

TAC *compile_math(NODE *tree)
{
  TAC *operand_one = compile(tree->left);
  TAC *operand_two = compile(tree->right);

  TAC *operation = new_tac(next_reg());
  operation->operation = tree->type;
  operation->operand_one = operand_one->destination;
  operation->operand_two = operand_two->destination;

  add_TAC_to_list(operand_two, operand_one);
  add_TAC_to_list(operation, operand_two);

  return operation;
}

TAC *compile_return(NODE *tree)
{
  TAC *operand_tac = compile(tree->left);

  TAC *return_tac = new_tac(next_reg());
  return_tac->operation = 'S';
  return_tac->operand_one = operand_tac->destination;
  return_tac->next = operand_tac;

  return return_tac;
}

TAC *compile_leaf(NODE *tree)
{
  if(tree->left->type == CONSTANT) {
    TOKEN *t = (TOKEN *)tree->left;
    LOCATION *loc = new_location(LOCTOKEN);
    loc->token = t;

    TAC *taccode = new_tac(next_reg());
    taccode->operation = 'S';
    taccode->operand_one = loc;

    return taccode;
  }
}

TAC *compile(NODE *tree)
{
  printf("NEXT TREE\n");
  print_tree(tree);

  switch (tree->type) {
    case RETURN:
      return compile_return(tree);
    case LEAF:
      return compile_leaf(tree);
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
      return compile_math(tree);
  }
}
