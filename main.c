#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "TACstruct.h"
#include "MIPS.h"
#include "definitions.h"
#include "tac_compiler.h"
#include "debug.h"

#define ANSWERVALUE 254

extern int yydebug;
extern NODE* yyparse(char* fileName);
extern NODE* ans;
extern TOKEN** init_symbtable(void);

//MY STUFF
extern UNION* intepret_body(NODE *tree, FRAME *enviroment);
extern UNION* intepret(NODE *tree, FRAME *enviroment);
extern UNION* interpret_definition(NODE *tree, FRAME *enviroment);

CLOSURE* new_closure(NODE *ast, FRAME *enviroment, NODE *arguments)
{
  CLOSURE *ans = (CLOSURE*)malloc(sizeof(CLOSURE));

  ans->ast = ast;
  ans->enviroment = enviroment;
  ans->arguments = arguments;

  return ans;
}

void store_variable(FRAME *enviroment, TOKEN *token, UNION *value)
{
  VARIABLE *variable = new_variable(token, value);

  if(enviroment->value == NULL){
    enviroment->value = variable;
  } else {
    VARIABLE *current = (VARIABLE*)enviroment->value;
    if (current->token == token)
    {
      current->value = value;
      return;
    }
    while ((current->next != NULL)) {
      current = (VARIABLE*)current->next;
    }
    current->next = variable;
  }
}

UNION* lookup_variable(FRAME *enviroment, TOKEN *target_token)
{
  //Loop Through Frames
  while(enviroment->value != 0)
  {
    VARIABLE *pointer = (VARIABLE*)enviroment->value;
    //Loop through variables in frame
    while(pointer != 0)
    {
      UNION *result = (UNION*)pointer->value;
      TOKEN *token = (TOKEN*)pointer->token;

      if (pointer->token == target_token)
      {
        //return value once it has been found
        TOKEN *token = (TOKEN*)pointer->token;

        return pointer->value;
      }
      pointer = pointer->next;
    }
    enviroment = enviroment->next;
  }
}

UNION* intepret_condition(NODE *tree, FRAME *enviroment)
{
  UNION *condition_result = intepret_body(tree->left, enviroment);
  //If its an if else
  if (tree->right->type == ELSE) {
    if(condition_result->value)
    {
      return intepret_body(tree->right->left, enviroment);
    } else {
      return intepret_body(tree->right->right, enviroment);
    }
  }
  //if it is only an if
  if(condition_result->value)
  {
    return intepret_body(tree->right, enviroment);
  }
}

UNION* intepret_math(NODE *tree, FRAME *enviroment)
{
  UNION *left_result = intepret_body(tree->left, enviroment);
  UNION *right_result = intepret_body(tree->right, enviroment);

  int left_value = left_result->value;
  int right_value = right_result->value;

  int result_value = 0;

  switch (tree->type) {

    case '<':
      result_value = left_value < right_value;
      break;

    case '>':
      result_value = left_value > right_value;
      break;

    case LE_OP:
      result_value = left_value <= right_value;
      break;

    case GE_OP:
      result_value = left_value >= right_value;
      break;

    case EQ_OP:
      result_value = left_value == right_value;
      break;

    case NE_OP:
      result_value = left_value != right_value;
      break;

    case '*':
      result_value = left_value * right_value;
      break;

    case '/':
      result_value = left_value / right_value;
      break;

    case '+':
      result_value = left_value + right_value;
      break;

    case '-':
      result_value = left_value - right_value;
      break;
  }

  UNION *result = new_union(INT);
  result->value = result_value;

  return result;
}

VARIABLE* parse_arguments(NODE *arguments, NODE *values, FRAME *enviroment)
{
  if (arguments->type == '~')
  {
    TOKEN *idtoken = (TOKEN*)arguments->right->left;
    TOKEN *typetoken = (TOKEN*)arguments->left->left;

    TOKEN *valuetoken = (TOKEN*)values->left;

    if (valuetoken->type == IDENTIFIER)
    {
      UNION *union_lookup = lookup_variable(enviroment, valuetoken);
      UNION *arg = new_union(typetoken->type);
      arg->value = union_lookup->value;
      arg->closure = union_lookup->closure;
      return new_variable(idtoken, arg);
    } else if(valuetoken->type == CONSTANT) {
      UNION *arg = new_union(typetoken->type);
      arg->value = valuetoken->value;
      return new_variable(idtoken, arg);
    } else {
      UNION *arg = intepret_body(values, enviroment);
      return new_variable(idtoken, arg);
    }
  } else {
    VARIABLE *left = parse_arguments(arguments->left, values->left, enviroment);
    left->next = parse_arguments(arguments->right, values->right, enviroment);

    return left;
  }
}

UNION* intepret_apply(FRAME *enviroment, NODE *tree)
{
  UNION *result = new_union(0);

  if (tree->left->type == APPLY) {
    result = intepret_apply(enviroment, tree->left);
    printf("returned value %d\n", result->type);
  } else {
      TOKEN *token = (TOKEN*)tree->left->left;
      result = lookup_variable(enviroment, token);
  }



  CLOSURE *closure = result->closure;
  NODE *node = (NODE*)closure->ast;

  NODE *values = tree->right;
  NODE *arguments = node->left->right->right;


  FRAME *new_env = new_frame();

  //Traverse both arguments tree
  if (arguments != 0) {
    VARIABLE *parsed_arguments = parse_arguments(arguments, values, enviroment);
    new_env->value = parsed_arguments;
  }

  FRAME *defined_enviroment = closure->enviroment;

  new_env->next = defined_enviroment;

  UNION *endresult = intepret_body(node->right, new_env);

  return endresult;
}

UNION *interpret_assingment(NODE *tree, FRAME *enviroment)
{
  TOKEN *token = tree->left->left;
  UNION *result = intepret_body(tree->right, enviroment);

  store_variable(enviroment, token, result);
}

UNION *interpret_loop(NODE *tree, FRAME *enviroment)
{
  UNION *condition = intepret_body(tree->left, enviroment);

  while (condition->value)
  {
    intepret_body(tree->right, enviroment);
    condition = intepret_body(tree->left, enviroment);
    printf("RESULT - %d\n", condition->value);
  }

  return;
}

UNION* intepret_body(NODE *tree, FRAME *enviroment)
{
  printf("NEXT TREE\n");

  print_tree(tree);

  //Find the type of the node and do something appropriate
  UNION *result = new_union(0);

  switch (tree->type) {
    case RETURN:
      result = intepret_body(tree->left, enviroment);
      result->hasreturned = 1;

      return result;

    case APPLY:
      return intepret_apply(enviroment, tree);

    case IF:
      return intepret_condition(tree, enviroment);
    case '=':
      return interpret_assingment(tree, enviroment);

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
      return intepret_math(tree, enviroment);

    case '~':
      if (tree->left->left->type == INT) {
        TOKEN *token = (TOKEN*)tree->right->left->left;

        UNION *result = new_union(INT);
        result->value = ((TOKEN*)tree->right->right->left)->value;

        store_variable(enviroment, token, result);
      }
      return;
    case 'D':
      return interpret_definition(tree, enviroment);
    case WHILE:
      return interpret_loop(tree, enviroment);
    case LEAF:
      if(tree->left->type == CONSTANT) {
        TOKEN *t = (TOKEN *)tree->left;

        result->value = t->value;

        return result;
      }
      if(tree->left->type == IDENTIFIER) {
        TOKEN *t = (TOKEN *)tree->left;

        UNION *stored_value = (UNION*)lookup_variable(enviroment, t);

        return stored_value;
      }
  }

  if (tree->type = ';') {
    result = intepret_body(tree->left, enviroment);

    if ((result) && (result->hasreturned == 1)) {
      return result;
    }

    result = intepret_body(tree->right, enviroment);
    if ((result) && (result->hasreturned == 1)) {
      return result;
    }
  }
}

UNION* interpret_definition(NODE *tree, FRAME *enviroment)
{
  //print_variable_list(enviroment->value);
  NODE *ast = tree;
  TOKEN *token = (TOKEN*)tree->left->right->left->left;
  NODE *arguments = tree->left->right->right;

  CLOSURE *closure = new_closure(tree, enviroment, arguments);

  UNION *function = new_union(FUNCTION);
  function->closure = closure;

  store_variable(enviroment, token, function);
}

UNION* intepret(NODE *tree, FRAME *enviroment)
{
  if (tree->type == '~') {
    UNION *result = intepret(tree->left, enviroment);
    if (result != 0) {
      return result;
    }
    return intepret(tree->right, enviroment);

  } else if (tree->type == 'D') {
    TOKEN *token = (TOKEN*)tree->left->right->left->left;
    char *funcname = token->lexeme;

    if (strcmp("main", token->lexeme) == 0) {
      UNION *result = intepret_body(tree->right, enviroment);
      return result;
    }

    interpret_definition(tree, enviroment);

    return 0;
  }
}

int main(int argc, char** argv)
{
    char* fileName = "";
    NODE* tree;
    int compileset = 0;

    if (strcmp(argv[1], "-c") == 0) {
      compileset = 1;
    }

    fileName = argv[2];

    if (argc>3 && strcmp(argv[3],"-d")==0) yydebug = 1;

    init_symbtable();
    printf("--C COMPILER\n");
    yyparse(fileName);
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);

    if (!compileset)
    {
      printf("Starting Interpretation\n");

      FRAME *enviroment = new_frame();
      UNION *result = (UNION*)intepret(tree, enviroment);

      printf("\nRESULT: %d\n", result->value);


      return result->value;
    } else {
      TAC *taccode = compile(tree);
      printf("COMPILED TO TAC\n");
      print_tac(taccode);
      printf("TRANSLATE TO MIPS\n");
      MIPS *ins = translate_tac(taccode);
      //This will need to change when functions are implemented
      FILE *file = fopen("Output/test.asm", "w");
      fprintf(file, ".globl main\n\n.text\n\n");
      print_mips(ins, file);
      print_mips(ins, 0);
      fprintf(file, "\n.data\n");

    }
}
