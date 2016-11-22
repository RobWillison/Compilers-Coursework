#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "definitions.h"
#include "TACstruct.h"
#include "MIPS.h"
#include "instructionSet.h"
#include "tacBlock.h"
#include "debug.h"

extern void compile_tree(NODE *tree);

typedef struct TAC_FRAME {
  struct TOKEN_INSTANCE *inst;
  struct FUNCTION_DEFINITION *definitions;
  struct TAC_FRAME *prev;
} TAC_FRAME;

typedef struct TOKEN_INSTANCE {
  struct TOKEN *token;
  struct TOKEN_INSTANCE *next;
} TOKEN_INSTANCE;

typedef struct FUNCTION_DEFINITION {
  struct TOKEN *token;
  int value;
  struct FUNCTION_DEFINITION *next;
} FUNCTION_DEFINITION;

int current_reg = 0;
int current_label = 0;
int current_function = 1;

TAC_FRAME *tac_env = NULL;

TOKEN_INSTANCE *addToCurrentScope(TOKEN *new_token)
{
  TOKEN_INSTANCE *new_instance = (TOKEN_INSTANCE*)malloc(sizeof(TOKEN_INSTANCE));
  new_instance->token = new_token;

  TOKEN_INSTANCE *instance = tac_env->inst;
  if (!instance)
  {
    tac_env->inst = new_instance;
    return new_instance;
  }

  while (instance->next) instance = instance->next;

  instance->next = new_instance;
  return new_instance;
}


int generateFunctionName(TOKEN *token)
{
  TOKEN_INSTANCE *new_instance = addToCurrentScope(token);
  FUNCTION_DEFINITION *newDefinition = (FUNCTION_DEFINITION*)malloc(sizeof(FUNCTION_DEFINITION));
  newDefinition->token = token;

  if (strcmp("main", token->lexeme) == 0)
  {
    newDefinition->value = 0;
  } else {
    newDefinition->value = current_function++;
  }

  FUNCTION_DEFINITION *definition = tac_env->definitions;

  if (!definition)
  {
    tac_env->definitions = newDefinition;
    return newDefinition->value;
  }

  while (definition->next) definition = definition->next;
  definition->next = newDefinition;

  return newDefinition->value;
}

int getFunctionName(TOKEN *look_token)
{
  TAC_FRAME *env = tac_env;

  while(env)
  {
    FUNCTION_DEFINITION *definition = env->definitions;

    while (definition)
    {
      if (definition->token == look_token)
      {
        return definition->value;
      }
      definition = definition->next;
    }

    env = env->prev;
  }

  return -1;
}

//returns the number of scopes up the token is defined
int whereIsTokenDefined(TOKEN *look_token)
{
  int scopes = 0;
  TAC_FRAME *env = tac_env;
  if (look_token->type == CONSTANT) return 0;

  while(env)
  {
    TOKEN_INSTANCE *instance = env->inst;

    while (instance)
    {
      if (instance->token == look_token) return scopes;
      instance = instance->next;
    }
    scopes++;
    env = env->prev;
  }

  return 0;
}

int get_label()
{
  current_label += 1;
  return current_label;
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
  compile_tree(tree->left);
  LOCATION *operand_one_location = getLastInstructionDestination();
  compile_tree(tree->right);
  LOCATION *operand_two_location = getLastInstructionDestination();

  TAC *operation = newTac();
  operation->destination = next_reg();
  operation->operation = tree->type;

  operation->operand_one = operand_one_location;
  operation->operand_two = operand_two_location;
}

void compile_return(NODE *tree)
{
  compile_tree(tree->left);
  LOCATION *return_location = getLastInstructionDestination();

  TAC *return_tac = newTac();
  return_tac->operation = RETURN;

  return_tac->operand_one = return_location;
}

void compile_leaf(NODE *tree)
{
  TOKEN *t = (TOKEN *)tree->left;
  LOCATION *loc = new_location(LOCTOKEN);
  loc->token = t;

  if (t->type == IDENTIFIER)
  {
    int functionName = getFunctionName(t);

    if (functionName != -1)
    {
      loc = new_location(LOCCLOSURE);
      loc->value = functionName;
    }
  }

  int definedScope = whereIsTokenDefined(t);

  TAC *taccode = newTac();
  taccode->destination = next_reg();
  taccode->operation = 'S';
  taccode->operand_one = loc;

  LOCATION *scope = new_location(LOCVALUE);
  scope->value = definedScope;

  taccode->operand_two = scope;
}

void compile_declaration(NODE *tree)
{
  LOCATION *destination = new_location(LOCTOKEN);

  destination->token = (TOKEN*)tree->right->left->left;
  if (tree->right->type == LEAF) destination->token = (TOKEN*)tree->right->left;


  addToCurrentScope(destination->token);

  LOCATION *operation_destination = new_location(LOCVALUE);
  operation_destination->value = 0;

  if(tree->right->right)
  {
    compile_tree(tree->right->right);
    operation_destination = getLastInstructionDestination();
  }


  TAC *taccode = newTac();
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

    TAC *taccode = newTac();
    taccode->destination = destination;
    taccode->operation = 'S';
    taccode->operand_one = operand;
  } else {
    LOCATION *destination = new_location(LOCTOKEN);
    destination->token = (TOKEN*)tree->left->left;

    compile_tree(tree->right);
    LOCATION *operation_destination = getLastInstructionDestination();
    TAC *taccode = newTac();
    taccode->destination = destination;
    taccode->operation = 'S';
    taccode->operand_one = operation_destination;
  }
}

void compile_conditional(NODE *tree)
{
  compile_tree(tree->left);
  LOCATION *condition_destination = getLastInstructionDestination();

  TAC *if_statement = newTac();
  if_statement->operation = IF_NOT;
  if_statement->operand_one = condition_destination;
  LOCATION *label = new_location(LABEL);
  label->value = get_label();
  if_statement->operand_two = label;

  newBlock();

  if (tree->right->type != ELSE)
  {
    compile_tree(tree->right);
    newBlock();
    TAC *label_tac = newTac();
    label_tac->operation = LABEL;
    label_tac->label = label->value;

    return;
  }

  compile_tree(tree->right->left);

  TAC *jump_to_end = newTac();
  jump_to_end->operation = JUMP;
  LOCATION *end_label = new_location(LABEL);
  end_label->value = get_label();
  jump_to_end->operand_one = end_label;

  newBlock();

  TAC *label_tac = newTac();
  label_tac->operation = LABEL;
  label_tac->label = label->value;

  compile_tree(tree->right->right);

  newBlock();

  TAC *end_label_tac = newTac();
  end_label_tac->operation = LABEL;
  end_label_tac->label = end_label->value;
}

void compile_while(NODE *tree)
{
  TAC *jump_to_end = newTac();
  jump_to_end->operation = JUMP;
  LOCATION *end_label = new_location(LABEL);
  end_label->value = get_label();
  jump_to_end->operand_one = end_label;

  LOCATION *start_label = new_location(LABEL);
  start_label->value = get_label();

  TAC *label_start = newTac();
  label_start->operation = LABEL;
  label_start->label = start_label->value;

  compile_tree(tree->right);

  TAC *label_end = newTac();
  label_end->operation = LABEL;
  label_end->label = end_label->value;


  compile_tree(tree->left);
  LOCATION *condition_destination = getLastInstructionDestination();

  TAC *if_statement = newTac();
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
      TAC* arg = newTac();
      arg->operation = LOADPARAM;
      LOCATION *token = new_location(LOCTOKEN);
      token->token = (TOKEN*)tree->right->left;
      arg->destination = token;

      addToCurrentScope(token->token);
    } else {
      create_load_arg(tree->left);
      create_load_arg(tree->right);
    }
  }
}

void compile_funcion_def(NODE *tree)
{
  TOKEN *functionToken = (TOKEN*)tree->left->right->left->left;
  int functionName = generateFunctionName(functionToken);

  TAC *define_closure = newTac();
  define_closure->operation = CREATE_CLOSURE;
  LOCATION *func_name = new_location(LOCCLOSURE);
  func_name->value = functionName;
  define_closure->operand_one = func_name;

  addFunctionBlock(functionName);

  addToCurrentScope(func_name->token);


  TAC_FRAME *new_frame = (TAC_FRAME*)malloc(sizeof(TAC_FRAME));
  new_frame->prev = tac_env;
  tac_env = new_frame;

  TAC *function = newTac();
  function->operation = FUNCTION_DEF;
  LOCATION *location = new_location(LOCCLOSURE);
  location->value = functionName;
  function->operand_one = location;

  TAC *frame = newTac();

  //Allocate Parameters to activation record
  //for each arguments

  create_load_arg(tree->left->right->right);

  compile_tree(tree->right);

  FUNCTION_BLOCK *functionBlock = getFunctionBlock(functionName);

  int locals = countLocalsInBlock(functionBlock);
  LOCATION *loc_local = new_location(INT);
  loc_local->value = locals;
  int tempories = countTemporiesInBlock(functionBlock);
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
  int last_tac_op = getLastInstructionOperation();
  if (last_tac_op != RETURN) {
    TAC *return_tac = newTac();
    return_tac->operation = RETURN;
  };


  TAC *end_tag = newTac();
  end_tag->operation = FUNC_END;

  tac_env = tac_env->prev;

  endFunctionBlock();
}

int count_parameters(NODE *tree)
{
  if (!tree) return 0;
  int count = 0;
  if (tree->type != ',')
  {
    count = count + 1;
  } else {
    count = count + count_parameters(tree->left);
    count = count + count_parameters(tree->right);
  }

  return count;
}

LOCATION **save_parameters(NODE *tree, LOCATION **parameter_locations)
{
  if (!tree) return parameter_locations;

  if (tree->type != ',')
  {
    compile_tree(tree);
    LOCATION *compile_destination = getLastInstructionDestination();

    int i = 0;
    while (parameter_locations[i] != NULL) i++;
    parameter_locations[i] = compile_destination;

  } else {
    save_parameters(tree->left, parameter_locations);
    save_parameters(tree->right, parameter_locations);
  }

  return parameter_locations;
}

void store_paraments(NODE *tree)
{
  int parameter_count = count_parameters(tree);
  LOCATION **parameter_location = (LOCATION**)calloc(parameter_count, sizeof(LOCATION));

  parameter_location = save_parameters(tree, parameter_location);

  TAC *parameter_setup = newTac();
  parameter_setup->operation = PARAMETER_ALLOCATE;
  LOCATION *param_count = new_location(LOCVALUE);
  param_count->value = parameter_count;
  parameter_setup->operand_one = param_count;

  int i;
  for (i = 0; i < parameter_count; i++)
  {
    TAC *save_param = newTac();
    save_param->operation = SAVE_PARAM;
    save_param->operand_one = parameter_location[i];
  }
}

void compile_apply(NODE *tree)
{
  if (tree->left->type == LEAF) {
    store_paraments(tree->right);

    //Allocate those bytes
    //Put values in
    //Put address in $a0
    //Put length in $a1
    TOKEN *functionToken = (TOKEN*)tree->left->left;
    int functionName = getFunctionName(functionToken);

    TAC *jump_to_func = newTac();
    jump_to_func->operation = JUMPTOFUNC;
    LOCATION *func_loc = new_location(LOCCLOSURE);
    func_loc->value = functionName;

    if (functionName == -1)
    {
      func_loc = new_location(LOCTOKEN);
      func_loc->token = functionToken;
    }

    newBlock();

    LOCATION *scope = new_location(LOCVALUE);
    scope->value = whereIsTokenDefined(functionToken);

    jump_to_func->operand_two = scope;

    jump_to_func->operand_one = func_loc;
    LOCATION *return_reg = new_location(LOCREG);
    return_reg->reg = RETURN_REG;
    jump_to_func->destination = return_reg;

    TAC *save_return = newTac();
    save_return->operation = 'S';
    save_return->destination = next_reg();
    save_return->operand_one = return_reg;
  } else {
    compile_tree(tree->left);
    LOCATION *function_location = getLastInstructionDestination();

    store_paraments(tree->right);

    LOCATION *scope = new_location(LOCVALUE);
    scope->value = 0;

    TAC *jump_to_func = newTac();
    jump_to_func->operation = JUMPTOFUNC;
    jump_to_func->operand_one = function_location;
    jump_to_func->operand_two = scope;

    newBlock();

    LOCATION *return_reg = new_location(LOCREG);
    return_reg->reg = RETURN_REG;
    jump_to_func->destination = return_reg;

    TAC *save_return = newTac();
    save_return->operation = 'S';
    save_return->destination = next_reg();
    save_return->operand_one = return_reg;
  }
}

void compile_tree(NODE *tree)
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
        compile_tree(tree->left);
        compile_tree(tree->right);
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
    compile_tree(tree->left);
    compile_tree(tree->right);
  }
}

TAC_BLOCK *compile(NODE *tree)
{
  startGlobalBlock();

  TAC_FRAME *global_frame = (TAC_FRAME*)malloc(sizeof(TAC_FRAME));
  tac_env = global_frame;

  compile_tree(tree);

  TAC_BLOCK *head = getHeadTacBlock();

  return head;
}
