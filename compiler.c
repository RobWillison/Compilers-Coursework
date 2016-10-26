#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "TACstruct.h"
#include "instructionSet.h"
#include "Memory.h"
#include "MIPS.h"
#include "definitions.h"
#include "debug.h"

FRAME *memory_env = NULL;

void add_MIPS_to_list(MIPS *front, MIPS *tail)
{
  while (front->next != 0) front = front->next;
  front->next = tail;
}

MIPS *new_mips()
{
  MIPS *ins = (MIPS*)malloc(sizeof(MIPS));
  return ins;
}

UNION* new_union(int type)
{
  UNION *ans = (UNION*)malloc(sizeof(UNION));
  ans->value = 0;
  ans->type = type;
  ans->hasreturned = 0;
  return ans;
}

FRAME* new_frame()
{
  FRAME *ans = (FRAME*)malloc(sizeof(FRAME));
  ans->next = 0;
  ans->value = 0;
  ans->memory = 0;
  return ans;
}

VARIABLE* new_variable(TOKEN *token, UNION *value)
{
  VARIABLE *ans = (VARIABLE*)malloc(sizeof(VARIABLE));
  ans->token = token;
  ans->value = value;
  ans->next = 0;
  return ans;
}

int store_in_memory(LOCATION *tac_location)
{
  int loc = 0;
  if (tac_location->type == LOCREG) {
    loc = tac_location->reg;
  } else {
    loc = tac_location->token;
  }

  //sav in lookup table
  MEMORY *memory = (MEMORY*)malloc(sizeof(MEMORY));
  memory->tac_location = loc;

  if(memory_env->memory == NULL){
    memory_env->memory = memory;
    memory->mips_location = 12;
  } else {
    MEMORY *current = (MEMORY*)memory_env->memory;
    while (current->next != NULL) {
      current = (MEMORY*)current->next;
    }
    memory->mips_location = current->mips_location + 4;
    current->next = memory;
  }

  return memory->mips_location;
}

int find_in_memory(LOCATION *tac_location)
{
  int target = 0;
  if (tac_location->type == LOCREG) {
    target = tac_location->reg;
  } else {
    target = tac_location->token;
  }

  FRAME *env_frame = memory_env;
  while (env_frame)
  {
    MEMORY *memory_frame = env_frame->memory;
    while (memory_frame != 0)
    {
      if (memory_frame->tac_location == target) return memory_frame->mips_location;
      memory_frame = memory_frame->next;
    }
    env_frame = env_frame->next;
  }


  return -1;
}

MIPS *create_activation_record(TAC *tac_code)
{
  int args = ((LOCATION*)tac_code->destination)->value;
  int locals = ((LOCATION*)tac_code->operand_one)->value;
  int tempories = ((LOCATION*)tac_code->operand_two)->value;

  int size = 4; //each arg, local or tempory size i.e. 4 byts
  //Work out how many bytes are needed
  int bytes_needed = (args + locals + tempories + 3) * size;//The 3 is the space for prev, enclosing and pc

  //Allocate Space
  MIPS *bytes = new_mips();
  bytes->instruction = LOADIMEDIATE_INS;
  bytes->destination = 4;
  bytes->operand_one = bytes_needed;

  MIPS *allocate = new_mips();
  allocate->instruction = LOADIMEDIATE_INS;
  allocate->destination = 2;
  allocate->operand_one = 9;
  allocate->next = bytes;

  MIPS *syscall = new_mips();
  syscall->instruction = SYSCALL;
  syscall->next = allocate;

  //Move $fp into $t0
  MIPS *move_fp = new_mips();
  move_fp->instruction = MOVE;
  move_fp->operand_two = 30;
  move_fp->operand_one = 8;
  move_fp->next = syscall;

  //Put $fp in prev and change $fp to the value in $v0
  MIPS *move_frame_pointer = new_mips();
  move_frame_pointer->instruction = MOVE;
  move_frame_pointer->operand_two = 2;
  move_frame_pointer->operand_one = 30;
  move_frame_pointer->next = move_fp;

  //Change prev value to that in $fp
  MIPS *store_instruction = new_mips();
  store_instruction->instruction = STOREWORD_FP;
  store_instruction->destination = 0;//First space in record (Previous)
  store_instruction->operand_one = 8;//$fp temp
  store_instruction->next = move_frame_pointer;

  //Store the $ra in the 2nd place in the frame
  MIPS *save_return = new_mips();
  save_return->instruction = STOREWORD_FP;
  save_return->destination = 4;//Second space in record
  save_return->operand_one = 31;//$ra
  save_return->next = store_instruction;

  return save_return;
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
  store_instruction->instruction = STOREWORD_FP;
  if (destination->type == LOCTOKEN) {
    store_instruction->destination = find_in_memory(destination);
    if (store_instruction->destination == -1)
    {
      store_instruction->destination = store_in_memory(tac_code->destination);
    }
  } else {
    store_instruction->destination = store_in_memory(tac_code->destination);
  }
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

  MIPS *store_instruction = new_mips();
  store_instruction->instruction = STOREWORD_FP;
  //Save the result
  int destination_register = find_in_memory(tac_code->destination);
  if (destination_register != -1) {
    store_instruction->destination = destination_register;
  } else {
    store_instruction->destination = store_in_memory(tac_code->destination);
  }

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
  store_instruction->instruction = STOREWORD_FP;
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
  store_instruction->instruction = STOREWORD_FP;
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
    eq_store_instruction->instruction = STOREWORD_FP;
    eq_store_instruction->destination = store_in_memory(tac_code->destination);
    eq_store_instruction->operand_one = or->destination;
    eq_store_instruction->next = or;

    return eq_store_instruction;
  }



  return store_instruction;
}

MIPS *translate_return(TAC *tac_code)
{
  //This method loads either an imediate value or memory value and stores it to
  //another memory location
  LOCATION *operand = tac_code->operand_one;
  LOCATION *destination = tac_code->destination;

  MIPS *load_instruction = new_mips();
  load_instruction->destination = MIPS_RETURN_REG;
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

    if (operand->reg == RETURN_REG){
      load_instruction->instruction = MOVE;
      load_instruction->operand_one = MIPS_RETURN_REG;
      load_instruction->operand_two = MIPS_RETURN_REG
    }
  }

  //Jump back to the return address
  //Load $ra out of the frame
  //Load two operands into registers
  MIPS *load_return_address = new_mips();
  load_return_address->instruction = LOADWORD_INS;
  load_return_address->destination = 8;
  load_return_address->operand_one = 4;
  load_return_address->next = load_instruction;
  //Move the $fp to the previous of the current $fp
  MIPS *restore_previous_frame = new_mips();
  restore_previous_frame->instruction = LOADWORD_INS;
  restore_previous_frame->destination = 30;
  restore_previous_frame->operand_one = 0;//prev frame pointer
  restore_previous_frame->next = load_return_address;
  //jump to the value
  MIPS *jump_to_return = new_mips();
  jump_to_return->instruction = JUMP_REG;
  jump_to_return->destination = 8;
  jump_to_return->next = restore_previous_frame;

  return jump_to_return;
}

MIPS *translate_conditional(TAC *tac_code)
{
  //Load two operands into registers
  MIPS *load_operand = new_mips();
  load_operand->instruction = LOADWORD_INS;
  load_operand->destination = 10;
  load_operand->operand_one = find_in_memory(tac_code->operand_one);

  MIPS *branch_instruction = new_mips();

  if (tac_code->operation == IF_NOT) {
    branch_instruction->instruction = BRANCH_EQ_INS;
  } else {
    branch_instruction->instruction = BRANCH_NEQ_INS;
  }

  LOCATION *label = tac_code->operand_two;
  branch_instruction->destination = label->value;
  branch_instruction->operand_one = load_operand->destination;
  branch_instruction->operand_two = 0;
  branch_instruction->next = load_operand;

  return branch_instruction;
}

MIPS *translate_label(TAC *tac_code)
{
  MIPS *label_instruction = new_mips();
  label_instruction->instruction = LABEL;
  label_instruction->operand_one = tac_code->label;

  return label_instruction;
}

MIPS *translate_function_def(TAC *tac_code)
{
  memory_env = new_frame();

  MIPS *label_instruction = new_mips();
  label_instruction->instruction = FUNCTION_DEF;
  label_instruction->operand_one = tac_code->operand_one;

  return label_instruction;
}

MIPS *translate_jump(TAC *tac_code)
{
  MIPS *jump_instruction = new_mips();
  jump_instruction->instruction = JUMP;
  LOCATION *operand_one = tac_code->operand_one;
  jump_instruction->operand_one = operand_one->value;

  return jump_instruction;
}

MIPS *translate_jump_to_func(TAC *tac_code)
{
  MIPS *jump_instruction = new_mips();
  jump_instruction->instruction = JUMPTOFUNC;
  LOCATION *operand_one = tac_code->operand_one;
  jump_instruction->operand_one = operand_one;

  return jump_instruction;
}

MIPS *allocate_space_for_params(TAC *tac_code)
{
  LOCATION *paramenter_count = tac_code->operand_one;
  int bytes_needed = paramenter_count->value * 4;

  MIPS *bytes = new_mips();
  bytes->instruction = LOADIMEDIATE_INS;
  bytes->destination = 4;
  bytes->operand_one = bytes_needed;

  MIPS *allocate = new_mips();
  allocate->instruction = LOADIMEDIATE_INS;
  allocate->destination = 2;
  allocate->operand_one = 9;
  allocate->next = bytes;

  MIPS *syscall = new_mips();
  syscall->instruction = SYSCALL;
  syscall->next = allocate;

  MIPS *save_pointer = new_mips();
  save_pointer->instruction = MOVE;
  save_pointer->operand_two = 2;
  save_pointer->operand_one = 9;
  save_pointer->next = syscall;

  return save_pointer;
}

MIPS *put_param_in_memory(TAC *tac_code)
{
  //load values
  MIPS *load_operand_one = new_mips();
  load_operand_one->instruction = LOADWORD_INS;
  load_operand_one->destination = 8;
  load_operand_one->operand_one = find_in_memory(tac_code->operand_one);
  //save in space
  MIPS *store_instruction = new_mips();
  store_instruction->instruction = STOREWORD_REG;

  store_instruction->destination = 9;
  store_instruction->operand_one = 8;//$fp temp
  store_instruction->next = load_operand_one;
  //increment space pointer
  MIPS *increment_pointer = new_mips();
  increment_pointer->instruction = ADD_IM;
  increment_pointer->destination = 9;
  increment_pointer->operand_one = 9;
  increment_pointer->operand_two = 4;
  increment_pointer->next = store_instruction;

  return increment_pointer;
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
    case RETURN:
      return translate_return(tac_code);
    case IF_NOT:
    case IF:
      return translate_conditional(tac_code);
    case LABEL:
      return translate_label(tac_code);
    case JUMP:
      return translate_jump(tac_code);
    case JUMPTOFUNC:
      return translate_jump_to_func(tac_code);
    case FUNCTION_DEF:
      return translate_function_def(tac_code);
    case NEWFRAME:
      return create_activation_record(tac_code);
    case PARAMETER_ALLOCATE:
      return allocate_space_for_params(tac_code);
    case SAVE_PARAM:
      return put_param_in_memory(tac_code);
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
