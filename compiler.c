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

extern MIPS *translate_tac_instruction();

MIPS_FRAME *mips_env = NULL;

typedef struct FUNCTION_NAME {
  struct TOKEN *token;
  int name;
  struct FUNCTION_NAME *next;
} FUNCTION_NAME;

FUNCTION_NAME *function_lookup_list = NULL;

int function_count = 0;

int find_func_name(TOKEN *function)
{
  FUNCTION_NAME *pointer = function_lookup_list;
  while(pointer)
  {
    if (function == pointer->token) return pointer->name;
    pointer = pointer->next;
  }

  FUNCTION_NAME *new_function = (FUNCTION_NAME*)malloc(sizeof(FUNCTION_NAME));
  new_function->token = function;
  new_function->name = ++function_count;
  new_function->next = function_lookup_list;
  function_lookup_list = new_function;

  return function_count;
}

MIPS *getEnclosingScope(int scopes)
{
  MIPS *initial = create_mips_instruction(MOVE, 0, 15, 30);
  MIPS *scope = initial;
  int i;
  for (i = 0; i < scopes; i++)
  {
    scope->next = create_mips_instruction(LOADWORD_INS, 15, 15, 8);
    scope = scope->next;
  }

  return initial;
}

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

MIPS_STORED_VALUE *get_location_from_env(LOCATION* location)
{
  TOKEN *token = location->token;

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

MIPS_CLOSURE *get_closure_from_env(LOCATION *closure)
{
  MIPS_STORED_VALUE *value = get_location_from_env(closure);
  if (!value) return NULL;
  return (MIPS_CLOSURE*)value->closure;
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
    return 12;
  }

  int last_used_memory = 12;
  while (binding)
  {
    MIPS_STORED_VALUE *stored_value = (MIPS_STORED_VALUE*)binding->value;

    if (stored_value->type == CONSTANT)
    {
      last_used_memory = ((MIPS_LOCATION*)stored_value->location)->memory_frame_location;
    } else {
      MIPS_CLOSURE *closure = (MIPS_CLOSURE*)stored_value->closure;
      last_used_memory = ((MIPS_LOCATION*)closure->enclosing_frame)->memory_frame_location;
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
  //Lookup name in function list
  int func_name = find_func_name(name);

  MIPS_CLOSURE *closure = (MIPS_CLOSURE*)malloc(sizeof(MIPS_CLOSURE));
  closure->name = func_name;
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

  //Put $fp in prev and change $fp to the value in $a0
  MIPS *move_frame_pointer = create_mips_instruction(MOVE, 0, 30, 2);
  move_fp->next = move_frame_pointer;

  //Change prev value to that in $fp
  MIPS *store_instruction = create_mips_instruction(STOREWORD, 30, 0, 8);
  move_frame_pointer->next = store_instruction;

  //Store the $ra in the 2nd place in the frame
  MIPS *save_return = create_mips_instruction(STOREWORD, 30, 4, 31);
  store_instruction->next = save_return;

  //move the enclosing frame into frame
  //Store the $ra in the 2nd place in the frame
  MIPS *store_enclosing = create_mips_instruction(STOREWORD, 30, 8, 5);
  save_return->next = store_enclosing;

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
      //Its a token do a lookup and load
      LOCATION *scope = tac_code->operand_two;
      int scopeDefined = scope->value;
      int framePointer = 30;
      MIPS *getEnclosing = NULL;

      if (scopeDefined)
      {
        getEnclosing = getEnclosingScope(scopeDefined);
        framePointer = 15;
      }

      load_instruction = create_mips_instruction(LOADWORD_INS, 8, framePointer, get_memory_location_from_env(operand));
      if (getEnclosing)
      {
        add_MIPS_to_list(getEnclosing, load_instruction);
        load_instruction = getEnclosing;
      }
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

  add_MIPS_to_list(load_instruction, store_instruction);

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
  TOKEN *token = (TOKEN*)((LOCATION*)tac_code->operand_one)->token;

  int function_name = find_func_name(token);

  MIPS *label_instruction = create_mips_instruction(FUNCTION_DEF, 0, function_name, 0);

  MIPS_FRAME *new_mips_env = (MIPS_FRAME*)malloc(sizeof(MIPS_FRAME));
  new_mips_env->prev = mips_env;
  new_mips_env->bindings = NULL;
  mips_env = new_mips_env;

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
  MIPS_CLOSURE *closure = get_closure_from_env(tac_code->operand_one);
  LOCATION *location = tac_code->operand_one;
  TOKEN *token = location->token;
  int name = find_func_name(token);

  MIPS *jump_instruction = create_mips_instruction(JUMPTOFUNC, 0, name, 0);

  if (closure) {
    MIPS_LOCATION *location = closure->enclosing_frame;
    MIPS *load_enclosing_scope = create_mips_instruction(LOADWORD_INS, 5, 30, location->memory_frame_location);
    load_enclosing_scope->next = jump_instruction;

    return load_enclosing_scope;
  }

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
  TOKEN *token = (TOKEN*)((LOCATION*)tac_code->operand_one)->token;

  MIPS_LOCATION *enclosing_frame = (MIPS_LOCATION*)malloc(sizeof(MIPS_LOCATION));
  enclosing_frame->type = INMEMORY;
  enclosing_frame->memory_frame_location = get_next_free_memory();

  store_closure(token, enclosing_frame);


  //Mips Save Value Of $fp
  MIPS *save_fp = create_mips_instruction(STOREWORD, 30, enclosing_frame->memory_frame_location, 30);

  return save_fp;
}

MIPS *create_global_scope(TAC *tac_code)
{

  MIPS *main_function = create_mips_instruction(FUNCTION_DEF, 0, MAIN_FUNC, 0);

  //Allocate Space
  MIPS *bytes = create_mips_instruction(LOADIMEDIATE_INS, 4, 3, 0); //COUNT NUMBER OF FUNCTIONS TODO
  bytes->operand_one = 24;
  main_function->next = bytes;

  MIPS *allocate = create_mips_instruction(LOADIMEDIATE_INS, 2, 9, 0);
  bytes->next = allocate;

  MIPS *syscall = create_mips_instruction(SYSCALL, 0, 0, 0);
  allocate->next = syscall;

  MIPS *move_frame_pointer = create_mips_instruction(MOVE, 0, 30, 2);
  syscall->next = move_frame_pointer;

  //Store the $ra in the 2nd place in the frame
  MIPS *save_return = create_mips_instruction(STOREWORD, 30, 4, 31);
  move_frame_pointer->next = save_return;

  //Call the users main function
  //Find in enviroment
  FUNCTION_NAME *pointer = function_lookup_list;
  int main_name;
  while (pointer)
  {
    TOKEN *token = (TOKEN*)pointer->token;
    if (strcmp(token->lexeme, "main") == 0)
    {
      main_name = pointer->name;
    }
    pointer = pointer->next;
  }

  MIPS *definitons = translate_tac_instruction(tac_code->next);
  tac_code->next = 0;

  save_return->next = definitons;

  MIPS *jump_instruction = create_mips_instruction(JUMPTOFUNC, 0, main_name, 0);
  add_MIPS_to_list(definitons, jump_instruction);

  MIPS *load_return_address = create_mips_instruction(LOADWORD_INS, 8, 30, 4);
  jump_instruction->next = load_return_address;
  //jump to the value
  MIPS *jump_to_return = create_mips_instruction(JUMP_REG, 8, 0, 0);
  load_return_address->next = jump_to_return;

  return main_function;
}

MIPS *check_if_at_end(TAC *tac_code)
{
  //At end of func so return to previous enviroment
  mips_env = mips_env->prev;

  //if true we are end of user code
  if (tac_code->next->operation == CREATE_CLOSURE)
  {
    //create main function and setup global scope
    return create_global_scope(tac_code);
  }

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
    case FUNC_END:
        return check_if_at_end(tac_code);
  }
}

MIPS *translate_tac_instruction(TAC *tac)
{
  if (!tac) return NULL;

  if (tac->next)
  {
    MIPS *front = tac_to_mips(tac);

    MIPS *tail = translate_tac_instruction(tac->next);

    if (front == NULL) return tail;
    if (tail == NULL) return front;
    add_MIPS_to_list(front, tail);

    return front;
  }

  return tac_to_mips(tac);
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
