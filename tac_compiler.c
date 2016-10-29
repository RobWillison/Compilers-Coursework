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

  TAC *tail_operand_one = get_tail(operand_one);
  TAC *tail_operand_two = get_tail(operand_two);

  operation->operand_one = tail_operand_one->destination;
  operation->operand_two = tail_operand_two->destination;

  add_TAC_to_list(operand_one, operand_two);
  add_TAC_to_list(operand_two, operation);

  return operand_one;
}

TAC *compile_return(NODE *tree)
{
  TAC *operand_tac = compile(tree->left);
  TAC *last_tac = get_tail(operand_tac);

  TAC *return_tac = new_tac();
  return_tac->operation = RETURN;
  TAC *tail_operand = get_tail(operand_tac);

  return_tac->operand_one = tail_operand->destination;
  add_TAC_to_list(operand_tac, return_tac);

  return operand_tac;
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

  TAC *tail_operation = get_tail(operation);
  taccode->operand_one = tail_operation->destination;
  tail_operation->next = taccode;
  return operation;
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
    TAC *tail_operation = get_tail(operation);
    TAC *taccode = new_tac();
    taccode->destination = destination;
    taccode->operation = 'S';
    taccode->operand_one = tail_operation->destination;

    tail_operation->next = taccode;

    return operation;
  }
}

TAC *compile_conditional(NODE *tree)
{
  TAC *condition = compile(tree->left);
  TAC *tail_condition = get_tail(condition);

  TAC *if_statement = new_tac();
  if_statement->operation = IF_NOT;
  if_statement->operand_one = tail_condition->destination;
  LOCATION *label = new_location(LABEL);
  label->value = get_label();
  if_statement->operand_two = label;

  tail_condition->next = if_statement;

  if (tree->right->type != ELSE)
  {
    TAC *body = compile(tree->right);
    add_TAC_to_list(if_statement, body);

    TAC *label_tac = new_tac();
    label_tac->operation = LABEL;
    label_tac->label = label->value;
    add_TAC_to_list(body, label_tac);

    return condition;
  }

  TAC *if_body = compile(tree->right->left);
  add_TAC_to_list(if_statement, if_body);

  TAC *jump_to_end = new_tac();
  jump_to_end->operation = JUMP;
  LOCATION *end_label = new_location(LABEL);
  end_label->value = get_label();
  jump_to_end->operand_one = end_label;
  add_TAC_to_list(if_body, jump_to_end);
  TAC *label_tac = new_tac();
  label_tac->operation = LABEL;
  label_tac->label = label->value;
  jump_to_end->next = label_tac;

  TAC *else_body = compile(tree->right->right);
  add_TAC_to_list(label_tac, else_body);

  TAC *end_label_tac = new_tac();
  end_label_tac->operation = LABEL;
  end_label_tac->label = end_label->value;
  add_TAC_to_list(else_body, end_label_tac);

  return condition;

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
  jump_to_end->next = label_start;

  TAC *body = compile(tree->right);

  add_TAC_to_list(label_start, body);

  TAC *label_end = new_tac();
  label_end->operation = LABEL;
  label_end->label = end_label->value;
  add_TAC_to_list(body, label_end);


  TAC *condition = compile(tree->left);
  add_TAC_to_list(label_end, condition);

  TAC *if_statement = new_tac();
  if_statement->operation = IF;
  TAC *tail_condition = get_tail(condition);
  if_statement->operand_one = tail_condition->destination;
  if_statement->operand_two = start_label;
  add_TAC_to_list(condition, if_statement);

  return jump_to_end;
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

TAC *create_load_arg(NODE *tree)
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
      return arg;
    } else {
      TAC *left = create_load_arg(tree->left);
      left->next = create_load_arg(tree->right);
      return left;
    }
  }

  return NULL;
}

TAC *compile_funcion_def(NODE *tree)
{

  TAC *function = new_tac();
  function->operation = FUNCTION_DEF;
  LOCATION *location = new_location(LOCTOKEN);
  location->token = (TOKEN*)tree->left->right->left->left;
  function->operand_one = location;

  TAC *body = compile(tree->right);

  int locals = count_locals(body);
  LOCATION *loc_local = new_location(INT);
  loc_local->value = locals;
  int tempories = count_tempories(body);
  LOCATION *loc_temp = new_location(INT);
  loc_temp->value = tempories;
  int arguments = count_arguments(tree->left->right->right);
  LOCATION *loc_args = new_location(INT);
  loc_args->value = arguments;

  TAC *frame = new_tac();
  frame->operation = NEWFRAME;
  frame->destination = loc_args;
  frame->operand_one = loc_local;
  frame->operand_two = loc_temp;

  //Allocate Parameters to activation record
  //for each arguments
  TAC *load_arguments = create_load_arg(tree->left->right->right);
  if (load_arguments) {
    add_TAC_to_list(frame, load_arguments);
    add_TAC_to_list(load_arguments, body);
    function->next = frame;
  } else {
    add_TAC_to_list(frame, body);
    function->next = frame;
  }

  //If the last command in the body isnt a RETURN add one
  TAC *last_tac = get_tail(body);
  if (last_tac->operation == RETURN) return function;

  TAC *return_tac = new_tac();
  return_tac->operation = RETURN;
  body->next = return_tac;

  return function;
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

TAC *save_parameters(NODE *tree)
{
  if (!tree) return 0;

  if (tree->type == LEAF)
  {
    TAC *get_param = compile(tree);
    TAC *save_param = new_tac();
    save_param->operation = SAVE_PARAM;
    save_param->operand_one = get_param->destination;
    add_TAC_to_list(get_param, save_param);
    return get_param;
  } else {
    TAC *left_param = save_parameters(tree->left);
    TAC *right_param = save_parameters(tree->right);
    add_TAC_to_list(right_param, left_param);
    return right_param;
  }
}

TAC *store_paraments(NODE *tree)
{
  TAC *parameter_setup = new_tac();
  parameter_setup->operation = PARAMETER_ALLOCATE;
  LOCATION *param_count = new_location(LOCVALUE);
  param_count->value = count_parameters(tree);
  parameter_setup->operand_one = param_count;

  TAC *parameters = save_parameters(tree);
  add_TAC_to_list(parameter_setup, parameters);

  return parameter_setup;
}

TAC *compile_apply(NODE *tree)
{
  TAC *parameters = store_paraments(tree->right);
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

  add_TAC_to_list(parameters, jump_to_func);

  return parameters;
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
    case '~':
      if (tree->left->type == 'D')
      {
        TAC *first_func = compile(tree->left);
        TAC *second_func = compile(tree->right);
        add_TAC_to_list(first_func, second_func);
        return first_func;
      } else {
        return compile_declaration(tree);
      }
    case IF:
      return compile_conditional(tree);
    case '=':
      return compile_assignment(tree);
    case WHILE:
      return compile_while(tree);
    case 'D':
      return compile_funcion_def(tree);
    case APPLY:
      return compile_apply(tree);
  }

  if (tree->type == ';')
  {
    TAC *left = compile(tree->left);
    TAC *right = compile(tree->right);

    add_TAC_to_list(left, right);
    return left;
  }
}
