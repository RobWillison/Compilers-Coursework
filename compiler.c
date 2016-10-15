#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "debug.h"
#include "TACstruct.h"
#include "MIPS.h"
#include "instructionSet.h"
#include "Memory.h"

int current_reg = 0;
int current_memory = 0;
char *registers[] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t8"};
MEMORY *memory_env = NULL;

#define LOCTOKEN 5
#define LOCREG 4

extern TAC *compile(NODE *tree);

void add_TAC_to_list(TAC *front, TAC *tail)
{
  while (front->next != 0) front = front->next;
  front->next = tail;
}

void add_MIPS_to_list(MIPS *front, MIPS *tail)
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

char *get_instruction(int instruction)
{
  switch (instruction) {
    case LOADIMEDIATE_INS:
      return "li";
    case LOADWORD_INS:
      return "lw";
    case STOREWORD_INS:
      return "sw";
    case '+':
      return "add";
    case '-':
      return "sub";
    case '*':
      return "mult";
    case '/':
      return "div";
    case MOVE_LOW_INS:
      return "mflo";
    case SET_LESS_THAN_INS:
      return "sltu";
    case XOR_IMEDIATE_INS:
      return "xori";
    case OR_INS:
      return "or";
  }
}

void print_mips(MIPS *mips)
{
  if (mips->next) print_mips(mips->next);

  switch (mips->instruction) {
    case LOADIMEDIATE_INS:
      printf("%s %s %d\n", get_instruction(mips->instruction), registers[mips->destination], mips->operand_one);
      break;
    case STOREWORD_INS:
      printf("%s %s %d($sp)\n", get_instruction(mips->instruction), registers[mips->operand_one], mips->destination);
      break;
    case LOADWORD_INS:
      printf("%s %s %d($sp)\n", get_instruction(mips->instruction), registers[mips->destination], mips->operand_one);
      break;
    case '+':
    case '-':
    case SET_LESS_THAN_INS:
    case OR_INS:
      printf("%s %s %s %s\n", get_instruction(mips->instruction), registers[mips->destination], registers[mips->operand_one], registers[mips->operand_two]);
      break;
    case '*':
    case '/':
      printf("%s %s %s\n", get_instruction(mips->instruction), registers[mips->operand_one], registers[mips->operand_two]);
      break;
    case MOVE_LOW_INS:
      printf("%s %s\n", get_instruction(mips->instruction), registers[mips->destination]);
      break;
    case XOR_IMEDIATE_INS:
      printf("%s %s %s %d\n", get_instruction(mips->instruction), registers[mips->destination], registers[mips->operand_one], mips->operand_two);
      break;
  }
}

TAC *new_tac(LOCATION *destination)
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

MIPS *new_mips()
{
  MIPS *ins = (MIPS*)malloc(sizeof(MIPS));
  return ins;
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

  TOKEN *t = (TOKEN *)tree->left;
  LOCATION *loc = new_location(LOCTOKEN);
  loc->token = t;

  TAC *taccode = new_tac(next_reg());
  taccode->operation = 'S';
  taccode->operand_one = loc;

  return taccode;

}

TAC *compile_assignment(NODE *tree)
{
  LOCATION *destination = new_location(LOCTOKEN);
  destination->token = (TOKEN*)tree->right->left->left;
  TAC *operation = compile(tree->right->right);
  TAC *taccode = new_tac(destination);
  taccode->operation = 'S';
  taccode->operand_one = operation->destination;

  taccode->next = operation;
  return taccode;
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
      return compile_assignment(tree);
  }

  if (tree->type == ';')
  {
    TAC *left = compile(tree->left);
    TAC *right = compile(tree->right);
    add_TAC_to_list(right, left);
    return right;
  }
}

int store_in_memory(LOCATION *tac_location)
{
  int loc = 0;
  if (tac_location->type == LOCREG) {
    loc = tac_location->reg;
  } else {
    loc = tac_location->token;
  }

  int location = current_memory;
  current_memory += 4;

  //sav in lookup table
  MEMORY *memory = (MEMORY*)malloc(sizeof(MEMORY));
  memory->next = memory_env;
  memory_env = memory;

  memory->mips_location = location;
  memory->tac_location = loc;

  return location;
}

int find_in_memory(LOCATION *tac_location)
{
  int target = 0;
  if (tac_location->type == LOCREG) {
    target = tac_location->reg;
  } else {
    target = tac_location->token;
  }

  MEMORY *memory_frame = memory_env;
  while (memory_frame != 0)
  {
    if (memory_frame->tac_location == target) return memory_frame->mips_location;
    memory_frame = memory_frame->next;
  }
}

MIPS *translate_store(TAC *tac_code)
{

  //This method loads either an imediate value or memory value and stores it to
  //another memory location
  LOCATION *operand = tac_code->operand_one;
  LOCATION *destination = tac_code->destination;

  MIPS *load_instruction = new_mips();
  load_instruction->destination = 8; //RANDOM register choice
  if (operand->type == LOCTOKEN)
  {
    //if its in a token i.e. value or variable
    TOKEN *token = operand->token;
    if (token->type == CONSTANT)
    {
      //Its a value do a load imediate
      load_instruction->instruction = LOADIMEDIATE_INS;
      load_instruction->operand_one = token->value;
    } else {
      //Its a value do a load imediate
      load_instruction->instruction = LOADWORD_INS;
      load_instruction->operand_one = find_in_memory(operand);
    }
  } else {
    //If its in a memory location
    load_instruction->instruction = LOADWORD_INS;
    load_instruction->operand_one = find_in_memory(tac_code->operand_one);
  }

  MIPS *store_instruction = new_mips();
  store_instruction->instruction = STOREWORD_INS;
  store_instruction->destination = store_in_memory(tac_code->destination);
  store_instruction->operand_one = load_instruction->destination;

  store_instruction->next = load_instruction;
  return store_instruction;
}

MIPS *translate_math(TAC *tac_code)
{
  //Load two operands into memory
  MIPS *load_operand_one = new_mips();
  load_operand_one->instruction = LOADWORD_INS;
  load_operand_one->destination = 9;
  load_operand_one->operand_one = find_in_memory(tac_code->operand_one);

  //Load two operands into registers
  MIPS *load_operand_two = new_mips();
  load_operand_two->instruction = LOADWORD_INS;
  load_operand_two->destination = 10;
  load_operand_two->operand_one = find_in_memory(tac_code->operand_two);
  //Do the math operation
  MIPS *math_instruction = new_mips();
  math_instruction->instruction = tac_code->operation;
  math_instruction->destination = 8;
  math_instruction->operand_one = load_operand_one->destination;
  math_instruction->operand_two = load_operand_two->destination;
  //Save the result
  MIPS *store_instruction = new_mips();
  store_instruction->instruction = STOREWORD_INS;
  store_instruction->destination = store_in_memory(tac_code->destination);
  store_instruction->operand_one = math_instruction->destination;

  if ((tac_code->operation == '*') || (tac_code->operation == '/'))
  {
    MIPS *move_from_low = new_mips();
    move_from_low->instruction = MOVE_LOW_INS;
    move_from_low->destination = math_instruction->destination;

    store_instruction->next = move_from_low;
    move_from_low->next = math_instruction;
    math_instruction->next = load_operand_two;
    load_operand_two->next = load_operand_one;

    return store_instruction;
  }

  store_instruction->next = math_instruction;
  math_instruction->next = load_operand_two;
  load_operand_two->next = load_operand_one;

  return store_instruction;
}

MIPS *translate_equality_check(TAC *tac_code)
{
  //Load two operands into memory
  MIPS *load_operand_one = new_mips();
  load_operand_one->instruction = LOADWORD_INS;
  load_operand_one->destination = 9;
  load_operand_one->operand_one = find_in_memory(tac_code->operand_one);

  //Load two operands into registers
  MIPS *load_operand_two = new_mips();
  load_operand_two->instruction = LOADWORD_INS;
  load_operand_two->destination = 10;
  load_operand_two->operand_one = find_in_memory(tac_code->operand_two);
  load_operand_two->next = load_operand_one;

  //Subtract one from the other
  MIPS *subract = new_mips();
  subract->instruction = '-';
  subract->operand_one = load_operand_one->destination;
  subract->operand_two = load_operand_two->destination;
  subract->destination = 8;
  subract->next = load_operand_two;
  //Check if the result is less than 0
  MIPS *less_than = new_mips();
  less_than->instruction = SET_LESS_THAN_INS;
  less_than->operand_one = 0; //register $zero
  less_than->operand_two = subract->destination;
  less_than->destination = 8;
  less_than->next = subract;

  //Save the result
  MIPS *store_instruction = new_mips();
  store_instruction->instruction = STOREWORD_INS;
  store_instruction->destination = store_in_memory(tac_code->destination);
  store_instruction->operand_one = less_than->destination;

  if (tac_code->operation == EQ_OP)
  {
    //As this gives 0 if equal and 1 otherwise we need to flip the LSB
    MIPS *xor = new_mips();
    xor->instruction = XOR_IMEDIATE_INS;
    xor->operand_one = less_than->destination;
    xor->operand_two = 1;
    xor->destination = 8;
    store_instruction->operand_one = xor->destination;

    xor->next = less_than;
    store_instruction->next = xor;

    return store_instruction;
  }

  //else its a !=
  store_instruction->next = less_than;

  return store_instruction;
}

MIPS *translate_logic(TAC *tac_code)
{
  //Load two operands into memory
  MIPS *load_operand_one = new_mips();
  load_operand_one->instruction = LOADWORD_INS;
  load_operand_one->destination = 9;
  load_operand_one->operand_one = find_in_memory(tac_code->operand_one);

  //Load two operands into registers
  MIPS *load_operand_two = new_mips();
  load_operand_two->instruction = LOADWORD_INS;
  load_operand_two->destination = 10;
  load_operand_two->operand_one = find_in_memory(tac_code->operand_two);
  load_operand_two->next = load_operand_one;

  //Check which is greater than
  MIPS *less_than = new_mips();
  less_than->instruction = SET_LESS_THAN_INS;
  less_than->operand_one = load_operand_one->destination;
  less_than->operand_two = load_operand_two->destination;

  //if its a greater than swap operands
  if (tac_code->operation == '>' || tac_code->operation == GE_OP) {
    less_than->operand_one = load_operand_two->destination;
    less_than->operand_two = load_operand_one->destination;
  }

  less_than->destination = 8;
  less_than->next = load_operand_two;

  //Save the result
  MIPS *store_instruction = new_mips();
  store_instruction->instruction = STOREWORD_INS;
  store_instruction->destination = store_in_memory(tac_code->destination);
  store_instruction->operand_one = less_than->destination;
  store_instruction->next = less_than;

  if (tac_code->operation == GE_OP || tac_code->operation == LE_OP)
  {
    tac_code->operation = EQ_OP;
    MIPS *equality = translate_equality_check(tac_code);
    equality = equality->next;
    equality->destination = 9;
    add_MIPS_to_list(equality, store_instruction);

    //Load less than check
    MIPS *load_less_than = new_mips();
    load_less_than->instruction = LOADWORD_INS;
    load_less_than->destination = 10;
    load_less_than->operand_one = store_instruction->destination;
    load_less_than->next = load_operand_one;
    load_less_than->next = equality;

    //Or the equality and less than
    MIPS *or = new_mips();
    or->instruction = OR_INS;
    or->destination = 8;
    or->operand_one = load_less_than->destination;
    or->operand_two = equality->destination;
    or->next = load_less_than;

    //Save the result
    MIPS *eq_store_instruction = new_mips();
    eq_store_instruction->instruction = STOREWORD_INS;
    eq_store_instruction->destination = store_in_memory(tac_code->destination);
    eq_store_instruction->operand_one = or->destination;
    eq_store_instruction->next = or;

    return eq_store_instruction;
  }



  return store_instruction;
}

MIPS *tac_to_mips(TAC *tac_code)
{
  switch (tac_code->operation) {
    case 'S':
      return translate_store(tac_code);
    case '+':
    case '-':
    case '*':
    case '/':
      return translate_math(tac_code);
    case EQ_OP:
    case NE_OP:
      return translate_equality_check(tac_code);
    case '>':
    case '<':
    case LE_OP:
    case GE_OP:
      return translate_logic(tac_code);
  }
}

MIPS *translate_tac(TAC *tac)
{
  if (tac->next)
  {
    MIPS *current = translate_tac(tac->next);
    MIPS *ins = tac_to_mips(tac);
    add_MIPS_to_list(ins, current);
    return ins;
  }

  return tac_to_mips(tac);

}
