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
  long loc = 0;
  if (tac_location->type == LOCREG) {
    loc = (long)tac_location->reg;
  } else {
    loc = (long)tac_location->token;
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
  long target = 0;
  if (tac_location->type == LOCREG) {
    target = (long)tac_location->reg;
  } else {
    target = (long)tac_location->token;
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

  int size = 4; //each arg, local or tempory size i.e. 4 bytes
  //Work out how many bytes are needed
  int bytes_needed = (args + locals + tempories + 3) * size;//The 3 is the space for prev, enclosing and pc

  //Allocate Space
  MIPS *bytes = create_mips_instruction(LOADIMEDIATE_INS, 4, bytes_needed, 0);
  bytes->operand_one = bytes_needed;

  MIPS *allocate = create_mips_instruction(LOADIMEDIATE_INS, 2, 9, 0);
  allocate->next = bytes;

  MIPS *syscall = create_mips_instruction(SYSCALL, 0, 0, 0);
  syscall->next = allocate;

  //Move $fp into $t0
  MIPS *move_fp = create_mips_instruction(MOVE, 0, 8, 30);
  move_fp->next = syscall;

  //Put $fp in prev and change $fp to the value in $v0
  MIPS *move_frame_pointer = create_mips_instruction(MOVE, 0, 30, 2);
  move_frame_pointer->next = move_fp;

  //Change prev value to that in $fp
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 30, 0, 8);
  store_instruction->next = move_frame_pointer;

  //Store the $ra in the 2nd place in the frame
  MIPS *save_return = create_mips_instruction(STOREWORD, 30, 4, 31);
  save_return->next = store_instruction;

  //Fill the arg slots in the activation record with arguments
  //$a0 contains a pointer to the arguments
  int i;
  MIPS *current_head = save_return;
  for (i = 0; i < args*4; i = i + 4)
  {
    //Read out of i($a0)
    MIPS *read_arg = create_mips_instruction(LOADWORD_INS, 9, 4, i);
    add_MIPS_to_list(read_arg, current_head);
    //save in 3 + i($fp)
    MIPS *save_arg = create_mips_instruction(STOREWORD, 30, i, 0);
    add_MIPS_to_list(save_arg, read_arg);
    current_head = save_arg;

    //save arg in enviroment
  }

  return current_head;
}

MIPS *translate_store(TAC *tac_code)
{
  //This method loads either an imediate value or memory value and stores it to
  //another memory location
  LOCATION *operand = tac_code->operand_one;
  LOCATION *destination = tac_code->destination;

  MIPS *load_instruction;
  if (operand->type == LOCTOKEN)
  {
    //if its in a token i.e. value or variable
    TOKEN *token = operand->token;
    if (token->type == CONSTANT)
    {
      //Its a value do a load imediate
      load_instruction = create_mips_instruction(LOADIMEDIATE_INS, 8, token->value, 0);
    } else {
      //Its a value do a load imediate
      load_instruction = create_mips_instruction(LOADWORD_INS, 8, 30, find_in_memory(operand));
    }
  } else {
    //If its in a memory location
    load_instruction = create_mips_instruction(LOADWORD_INS, 8, 30, find_in_memory(tac_code->operand_one));
  }

  MIPS *store_instruction;
  int in_memory = find_in_memory(destination);
  if (in_memory == -1) {
    int new_location = store_in_memory(tac_code->destination);
    store_instruction = create_mips_instruction(STOREWORD, 30, new_location, load_instruction->destination);
  } else {
    store_instruction = create_mips_instruction(STOREWORD, 30, in_memory, load_instruction->destination);
  }

  store_instruction->next = load_instruction;
  return store_instruction;
}

MIPS *translate_math(TAC *tac_code)
{
  //Load two operands into memory
  MIPS *load_operand_one = create_mips_instruction(LOADWORD_INS, 9, 30, find_in_memory(tac_code->operand_one));

  //Load two operands into registers
  MIPS *load_operand_two = create_mips_instruction(LOADWORD_INS, 10, 30, find_in_memory(tac_code->operand_two));

  //Do the math operation
  MIPS *math_instruction = create_mips_instruction(tac_code->operation, 8, load_operand_one->destination, load_operand_two->destination);

  MIPS *store_instruction;
  //Save the result
  int in_memory = find_in_memory(tac_code->destination);
  if (in_memory != -1) {
    store_instruction = create_mips_instruction(STOREWORD, 30, in_memory, math_instruction->destination);
  } else {
    store_instruction = create_mips_instruction(STOREWORD, 30, store_in_memory(tac_code->destination), math_instruction->destination);
  }

  if ((tac_code->operation == '*') || (tac_code->operation == '/'))
  {
    MIPS *move_from_low = create_mips_instruction(MOVE_LOW_INS, math_instruction->destination, 0, 0);

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
  MIPS *load_operand_one = create_mips_instruction(LOADWORD_INS, 9, 30, find_in_memory(tac_code->operand_one));

  //Load two operands into registers
  MIPS *load_operand_two = create_mips_instruction(LOADWORD_INS, 10, 30, find_in_memory(tac_code->operand_two));

  //Subtract one from the other
  MIPS *subract = create_mips_instruction('-', 8, load_operand_one->destination, load_operand_two->destination);
  subract->next = load_operand_two;

  //Check if the result is less than 0
  MIPS *less_than = create_mips_instruction(SET_LESS_THAN_INS, 8, 0, subract->destination);
  less_than->next = subract;

  //Save the result
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 30, store_in_memory(tac_code->destination), less_than->destination);

  if (tac_code->operation == EQ_OP)
  {
    //As this gives 0 if equal and 1 otherwise we need to flip the LSB
    MIPS *xor = create_mips_instruction(XOR_IMEDIATE_INS, 8, less_than->destination, 1);
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
  MIPS *load_operand_one = create_mips_instruction(LOADWORD_INS, 9, 30, find_in_memory(tac_code->operand_one));

  //Load two operands into registers
  MIPS *load_operand_two = create_mips_instruction(LOADWORD_INS, 10, 30, find_in_memory(tac_code->operand_two));
  load_operand_two->next = load_operand_one;

  //Check which is greater than
  MIPS *less_than = create_mips_instruction(SET_LESS_THAN_INS, 8, load_operand_one->destination, load_operand_two->destination);

  //if its a greater than swap operands
  if (tac_code->operation == '>' || tac_code->operation == GE_OP) {
    less_than->operand_one = load_operand_two->destination;
    less_than->operand_two = load_operand_one->destination;
  }

  less_than->next = load_operand_two;

  //Save the result
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 30, store_in_memory(tac_code->destination), less_than->destination);
  store_instruction->next = less_than;

  if (tac_code->operation == GE_OP || tac_code->operation == LE_OP)
  {
    tac_code->operation = EQ_OP;
    MIPS *equality = translate_equality_check(tac_code);
    equality = equality->next;
    equality->destination = 9;
    add_MIPS_to_list(equality, store_instruction);

    //Load less than check
    MIPS *load_less_than = create_mips_instruction(LOADWORD_INS, 10, 30, store_instruction->destination);
    load_less_than->next = load_operand_one;
    load_less_than->next = equality;

    //Or the equality and less than
    MIPS *or = create_mips_instruction(OR_INS, 8, load_less_than->destination, equality->destination);
    or->next = load_less_than;

    //Save the result
    MIPS *eq_store_instruction = create_mips_instruction(STOREWORD, 30, store_in_memory(tac_code->destination), or->destination);
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

  MIPS *load_instruction = 0;
  if (operand->reg != RETURN_REG){
    load_instruction = create_load_ins(destination, operand);
  }

  //Jump back to the return address
  //Load $ra out of the frame
  //Load two operands into registers
  MIPS *load_return_address = create_mips_instruction(LOADWORD_INS, 8, 30, 4);
  if (load_instruction) load_return_address->next = load_instruction;
  //Move the $fp to the previous of the current $fp
  MIPS *restore_previous_frame = create_mips_instruction(LOADWORD_INS, 30, 30, 0);
  restore_previous_frame->next = load_return_address;
  //jump to the value
  MIPS *jump_to_return = create_mips_instruction(JUMP_REG, 8, 0, 0);
  jump_to_return->next = restore_previous_frame;

  return jump_to_return;
}

MIPS *translate_conditional(TAC *tac_code)
{
  //Load two operands into registers
  MIPS *load_operand = create_mips_instruction(LOADWORD_INS, 10, 30, find_in_memory(tac_code->operand_one));

  LOCATION *label = tac_code->operand_two;
  MIPS *branch_instruction = create_mips_instruction(BRANCH_NEQ_INS, label->value, load_operand->destination, 0);

  if (tac_code->operation == IF_NOT) {
    branch_instruction->instruction = BRANCH_EQ_INS;
  }

  branch_instruction->next = load_operand;

  return branch_instruction;
}

MIPS *translate_label(TAC *tac_code)
{
  MIPS *label_instruction = create_mips_instruction(LABEL, 0, tac_code->label, 0);

  return label_instruction;
}

MIPS *translate_function_def(TAC *tac_code)
{
  memory_env = new_frame();

  MIPS *label_instruction = create_mips_instruction(FUNCTION_DEF, 0, (long)tac_code->operand_one, 0);

  return label_instruction;
}

MIPS *translate_jump(TAC *tac_code)
{
  LOCATION *operand_one = tac_code->operand_one;
  MIPS *jump_instruction = create_mips_instruction(JUMP, 0, operand_one->value, 0);

  return jump_instruction;
}

MIPS *translate_jump_to_func(TAC *tac_code)
{
  LOCATION *operand_one = tac_code->operand_one;
  MIPS *jump_instruction = create_mips_instruction(JUMPTOFUNC, 0, (long)operand_one, 0);

  return jump_instruction;
}

MIPS *allocate_space_for_params(TAC *tac_code)
{
  LOCATION *paramenter_count = tac_code->operand_one;
  int bytes_needed = paramenter_count->value * 4;

  MIPS *bytes = create_mips_instruction(LOADIMEDIATE_INS, 4, bytes_needed, 0);

  MIPS *allocate = create_mips_instruction(LOADIMEDIATE_INS, 2, 9, 0);
  allocate->next = bytes;

  MIPS *syscall = create_mips_instruction(SYSCALL, 0, 0, 0);
  syscall->next = allocate;

  MIPS *save_pointer_arguemnt = create_mips_instruction(MOVE, 0, 4, 2);
  save_pointer_arguemnt->next = syscall;

  return save_pointer_arguemnt;
}

MIPS *put_param_in_memory(TAC *tac_code)
{
  //load values
  MIPS *load_operand_one = create_mips_instruction(LOADWORD_INS, 8, 30, find_in_memory(tac_code->operand_one));

  //save in space
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 2, 0, 8);
  store_instruction->next = load_operand_one;

  //increment space pointer
  MIPS *increment_pointer = create_mips_instruction(ADD_IM, 2, 2, 4);
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
