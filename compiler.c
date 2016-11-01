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
#include "compiler.h"

MIPS_FRAME *mips_env = NULL;

int function_count = 0;

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

VARIABLE* new_variable(TOKEN *token, UNION *value)
{
  VARIABLE *ans = (VARIABLE*)malloc(sizeof(VARIABLE));
  ans->token = token;
  ans->value = value;
  ans->next = 0;
  return ans;
}

void add_binding_to_env(MIPS_BINDING *binding)
{
  MIPS_BINDING *pointer = mips_env->bindings;
  while(pointer->next) pointer = pointer->next;
  pointer->next = binding;
}

MIPS_STORED_VALUE *get_location_from_env(LOCATION* location)
{
  MIPS_FRAME *frame_pointer = mips_env;
  while (frame_pointer)
  {
    MIPS_BINDING *binding = frame_pointer->bindings;
    while (binding)
    {
      LOCATION *binding_loc = binding->tac_location;

      if ((location->type == LOCREG && binding_loc->reg == location->reg)
              || (location->type == LOCTOKEN && binding_loc->token == location->token))
      {
        return binding->value;
      }
      binding = binding->next;
    }

    frame_pointer = frame_pointer->prev;
  }

  return NULL;
}

int get_memory_location_from_env(LOCATION *location)
{
  MIPS_STORED_VALUE *result = get_location_from_env(location);

  if (result == NULL) return -1;
  return ((MIPS_LOCATION*)result->location)->memory_frame_location;
}

int get_next_free_memory()
{
  MIPS_BINDING *binding = mips_env->bindings;

  if (binding == NULL)
  {
    return 8;
  }

  int last_used_memory = 8;
  while (binding)
  {
    MIPS_STORED_VALUE *stored_value = (MIPS_STORED_VALUE*)binding->value;

    if (stored_value->type == CONSTANT)
    {
      last_used_memory = ((MIPS_LOCATION*)stored_value->location)->memory_frame_location;
    }
    binding = binding->next;
  }

  return last_used_memory + 4;
}

int store_value(LOCATION *tac_location)
{
  MIPS_BINDING *binding = mips_env->bindings;

  if (binding) {
    while (binding->next)
    {
      binding = binding->next;
    }
  }

  int memory_location = get_next_free_memory();

  MIPS_BINDING *new_binding = (MIPS_BINDING*)malloc(sizeof(MIPS_BINDING));
  new_binding->next = NULL;
  new_binding->tac_location = tac_location;
  if (binding) {
    binding->next = new_binding;
  } else {
    mips_env->bindings = new_binding;
  }

  MIPS_STORED_VALUE *new_value = (MIPS_STORED_VALUE*)malloc(sizeof(MIPS_STORED_VALUE));
  new_value->type = CONSTANT;
  new_binding->value = new_value;

  MIPS_LOCATION *new_location = (MIPS_LOCATION*)malloc(sizeof(MIPS_LOCATION));
  new_location->type = INMEMORY;
  new_location->memory_frame_location = memory_location;
  new_value->location = new_location;

  return memory_location;
}

void store_closure(TOKEN *name, MIPS_LOCATION *fp_location)
{
  MIPS_CLOSURE *closure = (MIPS_CLOSURE*)malloc(sizeof(MIPS_CLOSURE));
  closure->name = name->lexeme;
  closure->enclosing_frame = fp_location;

  MIPS_BINDING *binding = mips_env->bindings;

  if (binding) {
    while (binding->next)
    {
      binding = binding->next;
    }
  }

  int memory_location = get_next_free_memory();

  MIPS_BINDING *new_binding = (MIPS_BINDING*)malloc(sizeof(MIPS_BINDING));
  new_binding->next = NULL;
  LOCATION *tac_location = new_location(LOCTOKEN);
  tac_location->token = name;
  new_binding->tac_location = tac_location;
  if (binding) {
    binding->next = new_binding;
  } else {
    mips_env->bindings = new_binding;
  }

  MIPS_STORED_VALUE *new_value = (MIPS_STORED_VALUE*)malloc(sizeof(MIPS_STORED_VALUE));
  new_value->type = FUNCTION;
  new_value->closure = closure;
  new_binding->value = new_value;
}

MIPS *create_activation_record(TAC *tac_code)
{
  //TODO move the enclosing frame into frame

  int args = ((LOCATION*)tac_code->destination)->value;
  int locals = ((LOCATION*)tac_code->operand_one)->value;
  int tempories = ((LOCATION*)tac_code->operand_two)->value;

  //Move the argument in $ao for a pointer to the args into $t2
  MIPS *move_arg_pointer = create_mips_instruction(MOVE, 0, 10, 4);


  int size = 4; //each arg, local or tempory size i.e. 4 bytes
  //Work out how many bytes are needed
  int bytes_needed = (args + locals + tempories + 3) * size;//The 3 is the space for prev, enclosing and pc

  //Allocate Space
  MIPS *bytes = create_mips_instruction(LOADIMEDIATE_INS, 4, bytes_needed, 0);
  bytes->operand_one = bytes_needed;
  move_arg_pointer->next = bytes;

  MIPS *allocate = create_mips_instruction(LOADIMEDIATE_INS, 2, 9, 0);
  bytes->next = allocate;

  MIPS *syscall = create_mips_instruction(SYSCALL, 0, 0, 0);
  allocate->next = syscall;

  //Move $fp into $t0
  MIPS *move_fp = create_mips_instruction(MOVE, 0, 8, 30);
  syscall->next = move_fp;

  //Put $fp in prev and change $fp to the value in $v0
  MIPS *move_frame_pointer = create_mips_instruction(MOVE, 0, 30, 2);
  move_fp->next = move_frame_pointer;

  //Change prev value to that in $fp
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 30, 0, 8);
  move_frame_pointer->next = store_instruction;

  //Store the $ra in the 2nd place in the frame
  MIPS *save_return = create_mips_instruction(STOREWORD, 30, 4, 31);
  store_instruction->next = save_return;

  //for each load param which follows load the paramenter
  tac_code = tac_code->next;
  int count = 0;
  MIPS *current_tail = save_return;

  while (tac_code->operation == LOADPARAM)
  {

    MIPS *read_arg = create_mips_instruction(LOADWORD_INS, 9, 10, count * 4);
    add_MIPS_to_list(current_tail, read_arg);
    //save in 3 + i($fp)
    MIPS *save_arg = create_mips_instruction(STOREWORD, 30, store_value(tac_code->destination), 9);
    add_MIPS_to_list(read_arg, save_arg);
    current_tail = save_arg;

    count++;
    tac_code = tac_code->next;
  }

  return move_arg_pointer;
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
      load_instruction = create_mips_instruction(LOADWORD_INS, 8, 30, get_memory_location_from_env(operand));
    }
  } else {
    //If its in a memory location
    load_instruction = create_mips_instruction(LOADWORD_INS, 8, 30, get_memory_location_from_env(tac_code->operand_one));
  }

  MIPS *store_instruction;

  int in_memory = get_memory_location_from_env(destination);
  if (in_memory == -1) {
    int new_location = store_value(tac_code->destination);
    store_instruction = create_mips_instruction(STOREWORD, 30, new_location, load_instruction->destination);
  } else {
    store_instruction = create_mips_instruction(STOREWORD, 30, in_memory, load_instruction->destination);
  }

  load_instruction->next = store_instruction;

  return load_instruction;
}

MIPS *translate_math(TAC *tac_code)
{
  //Load two operands into memory
  MIPS *load_operand_one = create_mips_instruction(LOADWORD_INS, 9, 30, get_memory_location_from_env(tac_code->operand_one));

  //Load two operands into registers
  MIPS *load_operand_two = create_mips_instruction(LOADWORD_INS, 10, 30, get_memory_location_from_env(tac_code->operand_two));

  //Do the math operation
  MIPS *math_instruction = create_mips_instruction(tac_code->operation, 8, load_operand_one->destination, load_operand_two->destination);

  MIPS *store_instruction;
  //Save the result
  int in_memory = get_memory_location_from_env(tac_code->destination);
  if (in_memory != -1) {
    store_instruction = create_mips_instruction(STOREWORD, 30, in_memory, math_instruction->destination);
  } else {
    store_instruction = create_mips_instruction(STOREWORD, 30, store_value(tac_code->destination), math_instruction->destination);
  }

  if ((tac_code->operation == '*') || (tac_code->operation == '/'))
  {
    MIPS *move_from_low = create_mips_instruction(MOVE_LOW_INS, math_instruction->destination, 0, 0);

    load_operand_one->next = load_operand_two;
    load_operand_two->next = math_instruction;
    math_instruction->next = move_from_low;
    move_from_low->next = store_instruction;



    return load_operand_one;
  }
  load_operand_one->next = load_operand_two;
  load_operand_two->next = math_instruction;
  math_instruction->next = store_instruction;



  return load_operand_one;
}

MIPS *translate_equality_check(TAC *tac_code)
{
  //Load two operands into memory
  MIPS *load_operand_one = create_mips_instruction(LOADWORD_INS, 9, 30, get_memory_location_from_env(tac_code->operand_one));

  //Load two operands into registers
  MIPS *load_operand_two = create_mips_instruction(LOADWORD_INS, 10, 30, get_memory_location_from_env(tac_code->operand_two));
  load_operand_one->next = load_operand_two;
  //Subtract one from the other
  MIPS *subract = create_mips_instruction('-', 8, load_operand_one->destination, load_operand_two->destination);
  load_operand_two->next = subract;

  //Check if the result is less than 0
  MIPS *less_than = create_mips_instruction(SET_LESS_THAN_INS, 8, 0, subract->destination);
  subract->next = less_than;

  //Save the result
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 30, store_value(tac_code->destination), less_than->destination);

  if (tac_code->operation == EQ_OP)
  {
    //As this gives 0 if equal and 1 otherwise we need to flip the LSB
    MIPS *xor = create_mips_instruction(XOR_IMEDIATE_INS, 8, less_than->destination, 1);
    store_instruction->operand_one = xor->destination;

    less_than->next = xor;
    xor->next = store_instruction;

    return load_operand_one;
  }

  //else its a !=
  less_than->next = store_instruction;

  return load_operand_one;
}

MIPS *translate_logic(TAC *tac_code)
{
  //Load two operands into memory
  MIPS *load_operand_one = create_mips_instruction(LOADWORD_INS, 9, 30, get_memory_location_from_env(tac_code->operand_one));

  //Load two operands into registers
  MIPS *load_operand_two = create_mips_instruction(LOADWORD_INS, 10, 30, get_memory_location_from_env(tac_code->operand_two));
  load_operand_one->next = load_operand_two;

  //Check which is greater than
  MIPS *less_than = create_mips_instruction(SET_LESS_THAN_INS, 8, load_operand_one->destination, load_operand_two->destination);

  //if its a greater than swap operands
  if (tac_code->operation == '>' || tac_code->operation == GE_OP) {
    less_than->operand_one = load_operand_two->destination;
    less_than->operand_two = load_operand_one->destination;
  }

  load_operand_two->next = less_than;

  //Save the result
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 30, store_value(tac_code->destination), less_than->destination);
  less_than->next = store_instruction;

  if (tac_code->operation == GE_OP || tac_code->operation == LE_OP)
  {
    tac_code->operation = EQ_OP;
    MIPS *equality = translate_equality_check(tac_code);
    equality = equality->next;
    equality->destination = 9;
    add_MIPS_to_list(store_instruction, equality);

    //Load less than check
    MIPS *load_less_than = create_mips_instruction(LOADWORD_INS, 10, 30, store_instruction->destination);
    equality->next = load_less_than;

    //Or the equality and less than
    MIPS *or = create_mips_instruction(OR_INS, 8, load_less_than->destination, equality->destination);
    load_less_than->next = or;

    //Save the result
    MIPS *eq_store_instruction = create_mips_instruction(STOREWORD, 30, store_value(tac_code->destination), or->destination);
    or->next = eq_store_instruction;

    return load_operand_one;
  }

  return load_operand_one;
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

  //Move the $fp to the previous of the current $fp
  MIPS *restore_previous_frame = create_mips_instruction(LOADWORD_INS, 30, 30, 0);
  load_return_address->next = restore_previous_frame;
  //jump to the value
  MIPS *jump_to_return = create_mips_instruction(JUMP_REG, 8, 0, 0);
  restore_previous_frame->next = jump_to_return;

  if (load_instruction)
  {
    load_instruction->next = load_return_address;
    return load_instruction;
  }

  return load_return_address;
}

MIPS *translate_conditional(TAC *tac_code)
{
  //Load two operands into registers
  MIPS *load_operand = create_mips_instruction(LOADWORD_INS, 10, 30, get_memory_location_from_env(tac_code->operand_one));

  LOCATION *label = tac_code->operand_two;
  MIPS *branch_instruction = create_mips_instruction(BRANCH_NEQ_INS, label->value, load_operand->destination, 0);

  if (tac_code->operation == IF_NOT) {
    branch_instruction->instruction = BRANCH_EQ_INS;
  }

  load_operand->next = branch_instruction;

  return load_operand;
}

MIPS *translate_label(TAC *tac_code)
{
  MIPS *label_instruction = create_mips_instruction(LABEL, 0, tac_code->label, 0);

  return label_instruction;
}

MIPS *translate_function_def(TAC *tac_code)
{
  mips_env = mips_env->prev;

  MIPS_FRAME *new_mips_env = (MIPS_FRAME*)malloc(sizeof(MIPS_FRAME));
  new_mips_env->prev = mips_env;
  new_mips_env->bindings = NULL;
  mips_env = new_mips_env;



  MIPS *label_instruction = create_mips_instruction(FUNCTION_DEF, 0, ++function_count, 0);

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
  bytes->next = allocate;

  MIPS *syscall = create_mips_instruction(SYSCALL, 0, 0, 0);
  allocate->next = syscall;

  MIPS *save_pointer_arguemnt = create_mips_instruction(MOVE, 0, 4, 2);
  syscall->next = save_pointer_arguemnt;

  return bytes;
}

MIPS *put_param_in_memory(TAC *tac_code)
{
  //load values
  MIPS *load_operand_one = create_mips_instruction(LOADWORD_INS, 8, 30, get_memory_location_from_env(tac_code->operand_one));

  //save in space
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 2, 0, 8);
  load_operand_one->next = store_instruction;

  //increment space pointer
  MIPS *increment_pointer = create_mips_instruction(ADD_IM, 2, 2, 4);
  store_instruction->next = increment_pointer;

  return load_operand_one;
}

MIPS *create_closure(TAC *tac_code)
{
  //Mips Save Value Of $fp
  MIPS *save_fp = create_mips_instruction(STOREWORD, 30, store_value(tac_code->operand_one), 30);

  TOKEN *token = (TOKEN*)((LOCATION*)tac_code->operand_one)->token;

  MIPS_CLOSURE *closure = (MIPS_CLOSURE*)malloc(sizeof(MIPS_CLOSURE));
  closure->name = token->lexeme;

  return NULL;
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
    case LOADPARAM:
      return NULL;
    case CREATE_CLOSURE:
      return create_closure(tac_code);
  }
}

MIPS *setup_global_scope()
{

}

MIPS *translate_tac_instruction(TAC *tac)
{
  if (tac->next)
  {
    MIPS *front = tac_to_mips(tac);
    MIPS *tail = translate_tac_instruction(tac->next);

    if (!front) return tail;
    add_MIPS_to_list(front, tail);
    return front;
  }
}

MIPS *translate_tac(TAC *tac)
{
  if (mips_env == NULL)
  {
    mips_env = (MIPS_FRAME*)malloc(sizeof(MIPS_FRAME));
    mips_env->prev = 0;
    mips_env->bindings = NULL;
  }
  return translate_tac_instruction(tac);
}
