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

TAC *current_tac_tail;
TAC *current_tac_head;

TAC *get_tail(TAC* tac)
{
  if (tac->next) return get_tail(tac->next);

  return tac;
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

  if (!current_tac_tail) {
    current_tac_tail = tac_struct;
    current_tac_head = tac_struct;
  } else {
    current_tac_tail->next = tac_struct;
    current_tac_tail = tac_struct;

  }

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

void compile_math(NODE *tree)
{
  compile(tree->left);
  LOCATION *operand_one_location = current_tac_tail->destination;
  compile(tree->right);
  LOCATION *operand_two_location = current_tac_tail->destination;

  TAC *operation = new_tac();
  operation->destination = next_reg();
  operation->operation = tree->type;

  operation->operand_one = operand_one_location;
  operation->operand_two = operand_two_location;
}

void compile_return(NODE *tree)
{
  compile(tree->left);
  LOCATION *return_location = current_tac_tail->destination;

  TAC *return_tac = new_tac();
  return_tac->operation = RETURN;

  return_tac->operand_one = return_location;
}

void compile_leaf(NODE *tree)
{
  TOKEN *t = (TOKEN *)tree->left;
  LOCATION *loc = new_location(LOCTOKEN);
  loc->token = t;

  TAC *taccode = new_tac();
  taccode->destination = next_reg();
  taccode->operation = 'S';
  taccode->operand_one = loc;
}

void compile_declaration(NODE *tree)
{
  LOCATION *destination = new_location(LOCTOKEN);
  destination->token = (TOKEN*)tree->right->left->left;

  compile(tree->right->right);
  LOCATION *operation_destination = current_tac_tail->destination;

  TAC *taccode = new_tac();
  taccode->destination = destination;
  taccode->operation = 'S';

  taccode->operand_one = operation_destination;
}

void compile_assignment(NODE *tree)
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
  } else {
    LOCATION *destination = new_location(LOCTOKEN);
    destination->token = (TOKEN*)tree->left->left;

    compile(tree->right);
    LOCATION *operation_destination = current_tac_tail->destination;
    TAC *taccode = new_tac();
    taccode->destination = destination;
    taccode->operation = 'S';
    taccode->operand_one = operation_destination;
  }
}

void compile_conditional(NODE *tree)
{
  compile(tree->left);
  LOCATION *condition_destination = current_tac_tail->destination;

  TAC *if_statement = new_tac();
  if_statement->operation = IF_NOT;
  if_statement->operand_one = condition_destination;
  LOCATION *label = new_location(LABEL);
  label->value = get_label();
  if_statement->operand_two = label;


  if (tree->right->type != ELSE)
  {
    compile(tree->right);

    TAC *label_tac = new_tac();
    label_tac->operation = LABEL;
    label_tac->label = label->value;

    return;
  }

  compile(tree->right->left);

  TAC *jump_to_end = new_tac();
  jump_to_end->operation = JUMP;
  LOCATION *end_label = new_location(LABEL);
  end_label->value = get_label();
  jump_to_end->operand_one = end_label;

  TAC *label_tac = new_tac();
  label_tac->operation = LABEL;
  label_tac->label = label->value;

  compile(tree->right->right);

  TAC *end_label_tac = new_tac();
  end_label_tac->operation = LABEL;
  end_label_tac->label = end_label->value;
}

void compile_while(NODE *tree)
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

  compile(tree->right);

  TAC *label_end = new_tac();
  label_end->operation = LABEL;
  label_end->label = end_label->value;


  compile(tree->left);
  LOCATION *condition_destination = current_tac_tail->destination;

  TAC *if_statement = new_tac();
  if_statement->operation = IF;

  if_statement->operand_one = condition_destination;
  if_statement->operand_two = start_label;
}

int count_locals(TAC *tac_code)
{
  int count = 0;
  while (tac_code)
  {
    LOCATION *destination = tac_code->destination;

    if ((destination) && (destination->type == LOCTOKEN)) count += 1;
    tac_code = tac_code->next;
  }

  return count;
}

int count_tempories(TAC *tac_code)
{
  int count = 0;
  while (tac_code)
  {
    LOCATION *destination = tac_code->destination;

    if ((destination) && (destination->type == LOCREG)) count += 1;
    tac_code = tac_code->next;
  }

  return count;
}

int count_arguments(NODE *tree)
{
  int count = 0;
  if (tree)
  {
    if (tree->type == '~')
    {
      count = count + 1;
    } else {
      count = count + count_arguments(tree->right);
      count = count + count_arguments(tree->left);
    }
  }

  return count;
}

void create_load_arg(NODE *tree)
{
  if (tree)
  {
    if (tree->type == '~')
    {
      TAC* arg = new_tac();
      arg->operation = LOADPARAM;
      LOCATION *token = new_location(LOCTOKEN);
      token->token = (TOKEN*)tree->right->left;
      arg->destination = token;
    } else {
      create_load_arg(tree->left);
      create_load_arg(tree->right);
    }
  }
}

LOCATION *find_last_function_def()
{
  TAC *pointer = current_tac_head;
  LOCATION *last_func;
  while (pointer)
  {
    if (pointer->operation == FUNCTION_DEF) last_func = (LOCATION*)pointer->operand_one;
    pointer = pointer->next;
  }
  return last_func;
}

void compile_funcion_def(NODE *tree)
{
  TAC *tac_tail = current_tac_tail;
  TAC *tac_head = current_tac_head;

  current_tac_tail = 0;
  current_tac_head = 0;

  TAC *function = new_tac();
  function->operation = FUNCTION_DEF;
  LOCATION *location = new_location(LOCTOKEN);
  location->token = (TOKEN*)tree->left->right->left->left;
  function->operand_one = location;

  TAC *frame = new_tac();

  //Allocate Parameters to activation record
  //for each arguments
  create_load_arg(tree->left->right->right);

  TAC *body_start = current_tac_tail;
  compile(tree->right);
  //print_tac(body_start->next);

  int locals = count_locals(body_start->next);
  LOCATION *loc_local = new_location(INT);
  loc_local->value = locals;
  int tempories = count_tempories(body_start->next);
  LOCATION *loc_temp = new_location(INT);
  loc_temp->value = tempories;
  int arguments = count_arguments(tree->left->right->right);
  LOCATION *loc_args = new_location(INT);
  loc_args->value = arguments;


  frame->operation = NEWFRAME;
  frame->destination = loc_args;
  frame->operand_one = loc_local;
  frame->operand_two = loc_temp;

  //If the last command in the body isnt a RETURN add one
  TAC *last_tac = get_tail(body_start->next);
  if (last_tac->operation != RETURN) {
    TAC *return_tac = new_tac();
    return_tac->operation = RETURN;
  };

  current_tac_tail->next = tac_head;
  current_tac_tail = tac_tail;
}

int count_parameters(NODE *tree)
{
  if (!tree) return 0;
  int count = 0;
  if (tree->type == LEAF)
  {
    count = count + 1;
  } else {
    count = count + count_parameters(tree->left);
    count = count + count_parameters(tree->right);
  }

  return count;
}

void save_parameters(NODE *tree)
{
  if (!tree) return;

  if (tree->type == LEAF)
  {
    compile(tree);
    LOCATION *compile_destination = current_tac_tail->destination;

    TAC *save_param = new_tac();
    save_param->operation = SAVE_PARAM;
    save_param->operand_one = compile_destination;

  } else {
    save_parameters(tree->left);
    save_parameters(tree->right);
  }
}

void store_paraments(NODE *tree)
{
  TAC *parameter_setup = new_tac();
  parameter_setup->operation = PARAMETER_ALLOCATE;
  LOCATION *param_count = new_location(LOCVALUE);
  param_count->value = count_parameters(tree);
  parameter_setup->operand_one = param_count;

  save_parameters(tree);
}

void compile_apply(NODE *tree)
{

  store_paraments(tree->right);

  //Allocate those bytes
  //Put values in
  //Put address in $a0
  //Put length in $a1

  TAC *jump_to_func = new_tac();
  jump_to_func->operation = JUMPTOFUNC;
  LOCATION *func_loc = new_location(LOCTOKEN);
  func_loc->token = (TOKEN*)tree->left->left;
  jump_to_func->operand_one = func_loc;
  LOCATION *return_reg = new_location(LOCREG);
  return_reg->reg = RETURN_REG;
  jump_to_func->destination = return_reg;
}

TAC *compile(NODE *tree)
{
  printf("NEXT TREE\n");
  print_tree(tree);

  switch (tree->type) {
    case RETURN:
      compile_return(tree);
      break;
    case LEAF:
      compile_leaf(tree);
      break;
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
      compile_math(tree);
      break;
    case '~':
      if (tree->left->type == 'D')
      {
        compile(tree->left);
        compile(tree->right);
      } else {
        compile_declaration(tree);
      }
      break;
    case IF:
      compile_conditional(tree);
      break;
    case '=':
      compile_assignment(tree);
      break;
    case WHILE:
      compile_while(tree);
      break;
    case 'D':
      compile_funcion_def(tree);
      break;
    case APPLY:
      compile_apply(tree);
      break;
  }

  if (tree->type == ';')
  {
    compile(tree->left);
    compile(tree->right);
  }

  return current_tac_head;
}
