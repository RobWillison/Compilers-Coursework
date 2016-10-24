#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "definitions.h"
#include "TACstruct.h"
#include "MIPS.h"
#include "instructionSet.h"
#include "Memory.h"
#include "debug.h"

extern TAC *compile(NODE *tree);

int current_reg = 0;
int current_label = 0;

void add_TAC_to_list(TAC *front, TAC *tail)
{
  while (front->next != 0) front = front->next;
  front->next = tail;
}

int get_label()
{
  current_label += 1;
  return current_label;
}

TAC *new_tac()
{
  TAC *tac_struct = (TAC*)malloc(sizeof(TAC));
  tac_struct->destination = 0;
  tac_struct->next = 0;
  tac_struct->label = 0;
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

  TAC *operation = new_tac();
  operation->destination = next_reg();
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

  TAC *return_tac = new_tac();
  return_tac->operation = RETURN;
  return_tac->operand_one = operand_tac->destination;
  return_tac->next = operand_tac;

  return return_tac;
}

TAC *compile_leaf(NODE *tree)
{

  TOKEN *t = (TOKEN *)tree->left;
  LOCATION *loc = new_location(LOCTOKEN);
  loc->token = t;

  TAC *taccode = new_tac();
  taccode->destination = next_reg();
  taccode->operation = 'S';
  taccode->operand_one = loc;

  return taccode;

}

TAC *compile_declaration(NODE *tree)
{
  LOCATION *destination = new_location(LOCTOKEN);
  destination->token = (TOKEN*)tree->right->left->left;
  TAC *operation = compile(tree->right->right);
  TAC *taccode = new_tac();
  taccode->destination = destination;
  taccode->operation = 'S';
  taccode->operand_one = operation->destination;

  taccode->next = operation;
  return taccode;
}

TAC *compile_assignment(NODE *tree)
{
  if (tree->right->type == LEAF)
  {
    LOCATION *destination = new_location(LOCTOKEN);
    destination->token = (TOKEN*)tree->left->left;

    LOCATION *operand = new_location(LOCTOKEN);
    operand->token = (TOKEN*)tree->right->left;

    TAC *taccode = new_tac();
    taccode->destination = destination;
    taccode->operation = 'S';
    taccode->operand_one = operand;

    return taccode;
  } else {
    LOCATION *destination = new_location(LOCTOKEN);
    destination->token = (TOKEN*)tree->left->left;

    TAC *operation = compile(tree->right);

    TAC *taccode = new_tac();
    taccode->destination = destination;
    taccode->operation = 'S';
    taccode->operand_one = operation->destination;

    taccode->next = operation;

    return taccode;
  }
}

TAC *compile_conditional(NODE *tree)
{
  TAC *condition = compile(tree->left);
  TAC *if_statement = new_tac();
  if_statement->operation = IF_NOT;
  if_statement->operand_one = condition->destination;
  LOCATION *label = new_location(LABEL);
  label->value = get_label();
  if_statement->operand_two = label;
  if_statement->next = condition;

  if (tree->right->type != ELSE)
  {
    TAC *body = compile(tree->right);
    add_TAC_to_list(body, if_statement);

    TAC *label_tac = new_tac();
    label_tac->operation = LABEL;
    label_tac->label = label->value;
    label_tac->next = body;

    return label_tac;
  }

  TAC *if_body = compile(tree->right->left);
  add_TAC_to_list(if_body, if_statement);

  TAC *jump_to_end = new_tac();
  jump_to_end->operation = JUMP;
  LOCATION *end_label = new_location(LABEL);
  end_label->value = get_label();
  jump_to_end->operand_one = end_label;
  jump_to_end->next = if_body;

  TAC *label_tac = new_tac();
  label_tac->operation = LABEL;
  label_tac->label = label->value;
  label_tac->next = jump_to_end;

  TAC *else_body = compile(tree->right->right);
  add_TAC_to_list(else_body, label_tac);

  TAC *end_label_tac = new_tac();
  end_label_tac->operation = LABEL;
  end_label_tac->label = end_label->value;
  end_label_tac->next = else_body;

  return end_label_tac;

}

TAC *compile_while(NODE *tree)
{
  TAC *jump_to_end = new_tac();
  jump_to_end->operation = JUMP;
  LOCATION *end_label = new_location(LABEL);
  end_label->value = get_label();
  jump_to_end->operand_one = end_label;

  LOCATION *start_label = new_location(LABEL);
  start_label->value = get_label();

  TAC *label_start = new_tac();
  label_start->operation = LABEL;
  label_start->label = start_label->value;
  label_start->next = jump_to_end;

  TAC *body = compile(tree->right);

  add_TAC_to_list(body, label_start);

  TAC *label_end = new_tac();
  label_end->operation = LABEL;
  label_end->label = end_label->value;
  label_end->next = body;


  TAC *condition = compile(tree->left);
  add_TAC_to_list(condition, label_end);

  TAC *if_statement = new_tac();
  if_statement->operation = IF;
  if_statement->operand_one = condition->destination;
  if_statement->operand_two = start_label;
  if_statement->next = condition;

  return if_statement;
}

TAC *compile_funcion_def(NODE *tree)
{
  TAC *function = new_tac();
  function->operation = FUNCTION_DEF;
  LOCATION *location = new_location(LOCTOKEN);
  location->token = tree->left->right->left->left;
  function->operand_one = location;

  TAC *body = compile(tree->right);
  function->next = body;

  return function;
}

TAC *compile(NODE *tree)
{
  printf("NEXT TREE\n");

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
    case '~':
      return compile_declaration(tree);
    case IF:
      return compile_conditional(tree);
    case '=':
      return compile_assignment(tree);
    case WHILE:
      return compile_while(tree);
    case 'D':
      return compile_funcion_def(tree);
  }

  if (tree->type == ';')
  {
    TAC *left = compile(tree->left);
    TAC *right = compile(tree->right);

    add_TAC_to_list(right, left);
    return right;
  }
}
