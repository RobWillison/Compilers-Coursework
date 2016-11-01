#include <stdlib.h>
#include "C.tab.h"
#include "definitions.h"
#include "TACstruct.h"
#include "MIPS.h"
#include "instructionSet.h"

extern int get_memory_location_from_env(LOCATION *target);

const char *registers[] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

char *get_instruction(int instruction)
{
  switch (instruction) {
    case LOADIMEDIATE_INS:
      return "li";
    case LOADWORD_INS:
      return "lw";
    case STOREWORD:
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
    case BRANCH_EQ_INS:
      return "beq";
    case BRANCH_NEQ_INS:
      return "bne";
    case JUMP:
      return "j";
    case JUMPTOFUNC:
      return "jal";
    case MOVE:
      return "move";
    case JUMP_REG:
      return "jr";
    case ADD_IM:
      return "addi";
  }
}

MIPS *new_mips()
{
  MIPS *ins = (MIPS*)malloc(sizeof(MIPS));
  return ins;
}

MIPS *create_load_ins(LOCATION *destination, LOCATION *operand)
{
  MIPS *load_instruction = new_mips();
  load_instruction->destination = MIPS_RETURN_REG;
  load_instruction->operand_one = 30;
  if (operand->type == LOCTOKEN)
  {
    //if its in a token i.e. value or variable
    TOKEN *token = operand->token;
    if (token->type == CONSTANT)
    {
      //Its a value do a load imediate
      load_instruction->instruction = LOADIMEDIATE_INS;
      load_instruction->operand_two = token->value;
    } else {
      //Its a value do a load imediate
      load_instruction->instruction = LOADWORD_INS;
      load_instruction->operand_two = get_memory_location_from_env(operand);
    }
  } else {
    //If its in a memory location
    load_instruction->instruction = LOADWORD_INS;
    load_instruction->operand_two = get_memory_location_from_env(operand);
  }

  return load_instruction;
}

MIPS *create_mips_instruction(int type, int destination, int operand_one, int operand_two)
{
  MIPS *instruction = new_mips();
  if (type) instruction->instruction = type;
  if (destination) instruction->destination = destination;
  if (operand_one) instruction->operand_one = operand_one;
  if (operand_two) instruction->operand_two = operand_two;

  return instruction;
}
