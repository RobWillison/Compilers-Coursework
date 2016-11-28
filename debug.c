#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "definitions.h"
#include "C.tab.h"
#include <string.h>
#include "TACstruct.h"
#include "MIPS.h"
#include "instructionSet.h"
#include "tacBlock.h"


char *get_location(LOCATION *loc)
{
  if (loc->type == LOCREG)
  {
    if (loc->reg == RETURN_REG) return "result";
    char *string = malloc(sizeof(char) * 5);
    sprintf(string, "r%d", loc->reg);
    return string;
  } else if (loc->type == LOCTOKEN){
    TOKEN *t = (TOKEN*)loc->token;
    if (t->type == CONSTANT) {
      char *result = malloc(sizeof(char) * 3);
      sprintf(result, "%d", t->value);
      return result;
    } else {
      return t->lexeme;
    }
  } else {
    char *result = malloc(sizeof(char) * 10);
    sprintf(result, "%d", loc->value);
    return result;
  }
}

void print_variable_list(VARIABLE *pointer)
{
  printf("LIST\n");
  while (pointer != 0)
  {

    TOKEN *variable = (TOKEN*)pointer->token;
    UNION *union_result = (UNION*)pointer->value;

    if (union_result->type == INT) {
      printf("%s, %d\n", variable->lexeme, union_result->value);
    } else {
      printf("%s, CLOSURE\n", variable->lexeme);
    }
    pointer = pointer->next;
  }
  printf("END\n");
}

void print_enviroment(FRAME *enviroment)
{
  while (enviroment != 0)
  {
    printf("NEXT ENV FRAME\n");
    print_variable_list(enviroment->value);
    printf("END FRAME\n");

    enviroment = enviroment->next;
  }
  printf("END\n");
}

char *named(int t)
{
    static char b[100];
    if (isgraph(t) || t==' ') {
      sprintf(b, "%c", t);
      return b;
    }
    switch (t) {
      default: return "???";
    case IDENTIFIER:
      return "id";
    case CONSTANT:
      return "constant";
    case STRING_LITERAL:
      return "string";
    case LE_OP:
      return "<=";
    case GE_OP:
      return ">=";
    case EQ_OP:
      return "==";
    case NE_OP:
      return "!=";
    case EXTERN:
      return "extern";
    case AUTO:
      return "auto";
    case INT:
      return "int";
    case VOID:
      return "void";
    case APPLY:
      return "apply";
    case LEAF:
      return "leaf";
    case IF:
      return "if";
    case ELSE:
      return "else";
    case WHILE:
      return "while";
    case CONTINUE:
      return "continue";
    case BREAK:
      return "break";
    case RETURN:
      return "return";
    }
}

void print_leaf(NODE *tree, int level)
{
    TOKEN *t = (TOKEN *)tree;
    int i;

    for (i=0; i<level; i++) putchar(' ');
    if (t->type == CONSTANT) printf("%d\n", t->value);
    else if (t->type == STRING_LITERAL) printf("\"%s\"\n", t->lexeme);
    else if (t) puts(t->lexeme);
}

void print_tree0(NODE *tree, int level)
{

    int i;
    if (tree==NULL) return;
    if (tree->type==LEAF) {
      print_leaf(tree->left, level);
    }
    else {
      for(i=0; i<level; i++) putchar(' ');
      printf("%s\n", named(tree->type));
/*       if (tree->type=='~') { */
/*         for(i=0; i<level+2; i++) putchar(' '); */
/*         printf("%p\n", tree->left); */
/*       } */
/*       else */
        print_tree0(tree->left, level+2);
      print_tree0(tree->right, level+2);
    }
}

void print_tree(NODE *tree)
{
    print_tree0(tree, 0);
}

void print_tac(TAC *tac_code)
{
  if (tac_code == 0) return;

  if (tac_code->operation == 'S') {
    LOCATION *destination = tac_code->destination;
    LOCATION *operandOne = tac_code->operandOne;
    LOCATION *operandTwo = tac_code->operandTwo;
    if (operandTwo && operandTwo->value != 0)
    {
      int scope = operandTwo->value;
      printf("DEFINED IN %d ", scope);
    }
    printf("%s := %s\n", get_location(destination), get_location(operandOne));
  } else if (tac_code->operation == RETURN){
    if (tac_code->operandOne){
      LOCATION *operandOne = tac_code->operandOne;
      printf("RETURN %s\n", get_location(operandOne));
    } else { printf("RETURN\n"); }
  } else if (tac_code->operation == IF_NOT){
    LOCATION *operandOne = tac_code->operandOne;
    LOCATION *operandTwo = tac_code->operandTwo;
    printf("IF NOT %s GOTO %d\n", get_location(operandOne), operandTwo->value);
  } else if (tac_code->operation == IF){
    LOCATION *operandOne = tac_code->operandOne;
    LOCATION *operandTwo = tac_code->operandTwo;
    printf("IF %s GOTO %d\n", get_location(operandOne), operandTwo->value);
  } else if (tac_code->operation == LABEL){
    printf("LABEL %d: ", tac_code->label);
  } else if (tac_code->operation == JUMP){
    LOCATION *operandOne = tac_code->operandOne;
    printf("GOTO %d\n", operandOne->value);
  } else if (tac_code->operation == JUMPTOFUNC){
    LOCATION *operandOne = tac_code->operandOne;
    int scope = ((LOCATION*)tac_code->operandTwo)->value;
    if (operandOne->type == LOCREG)
    {
      printf("CALL %s FROM SCOPE %d\n", get_location(operandOne), scope);
    } else {
      printf("CALL _%d FROM SCOPE %d\n", operandOne->value, scope);
    }

  } else if (tac_code->operation == FUNCTION_DEF){
    LOCATION *location = tac_code->operandOne;
    printf("_%d:\n", location->value);
  } else if (tac_code->operation == CREATE_CLOSURE){
    LOCATION *location = tac_code->operandOne;
    printf("DEFINE CLOSURE _%d\n", location->value);
  } else if (tac_code->operation == NEWFRAME){
    LOCATION *arguments = tac_code->destination;
    LOCATION *locals = tac_code->operandOne;
    LOCATION *tempories = tac_code->operandTwo;

    printf("NEW FRAME %d arg %d loc %d temp\n", arguments->value, locals->value, tempories->value);
  } else if (tac_code->operation == PARAMETER_ALLOCATE){
    LOCATION *paramenter_count = tac_code->operandOne;

    printf("ALLOCATE PARAMS %d\n", paramenter_count->value);
  } else if (tac_code->operation == SAVE_PARAM){
    printf("SAVE PARAM %s\n", get_location(tac_code->operandOne));
  } else if (tac_code->operation == LOADPARAM){
    printf("LOAD PARAM %s\n", ((TOKEN*)((LOCATION*)tac_code->destination)->token)->lexeme);
  } else if (tac_code->operation == FUNC_END){
    printf("FUNCTION END\n");
  } else {
    LOCATION *destination = tac_code->destination;
    LOCATION *operandOne = tac_code->operandOne;
    LOCATION *operandTwo = tac_code->operandTwo;

    printf("%s := %s %s %s\n", get_location(destination), get_location(operandOne), named(tac_code->operation), get_location(operandTwo));
  }

  print_tac(tac_code->next);
}

void print_mips(MIPS *mips, FILE *file)
{
  if (!file)
  {
    file = stdout;
  }

  switch (mips->instruction) {
    case LOADIMEDIATE_INS:
      if (mips->destination >= 100){
        fprintf(file, "%s %d %d\n", get_instruction(mips->instruction), mips->destination, mips->operandOne);
      } else {
        fprintf(file, "%s %s %d\n", get_instruction(mips->instruction), registers[mips->destination], mips->operandOne);
      }
      break;
    case STOREWORD:
      fprintf(file, "%s %s %d(%s)\n", get_instruction(mips->instruction), registers[mips->operandTwo], mips->operandOne, registers[mips->destination]);
      break;
    case LOADWORD_INS:
      fprintf(file, "%s %s %d(%s)\n", get_instruction(mips->instruction), registers[mips->destination], mips->operandTwo, registers[mips->operandOne]);
      break;
    case '+':
    case '-':
    case SET_LESS_THAN_INS:
    case OR_INS:
      fprintf(file, "%s %s %s %s\n", get_instruction(mips->instruction), registers[mips->destination], registers[mips->operandOne], registers[mips->operandTwo]);
      break;
    case '*':
    case '/':
      fprintf(file, "%s %s %s\n", get_instruction(mips->instruction), registers[mips->operandOne], registers[mips->operandTwo]);
      break;
    case MOVE:
      if (mips->operandOne >= 100)
      {
        fprintf(file, "%s %d %s\n", get_instruction(mips->instruction), mips->destination, registers[mips->operandOne]);
      } else if (mips->operandTwo >= 100){
        fprintf(file, "%s %s %d\n", get_instruction(mips->instruction), registers[mips->destination], mips->operandOne);
      } else {
        fprintf(file, "%s %s %s\n", get_instruction(mips->instruction), registers[mips->destination], registers[mips->operandOne]);
      }
      break;
    case MOVE_LOW_INS:
    case JUMP_REG:
      fprintf(file, "%s %s\n", get_instruction(mips->instruction), registers[mips->destination]);
      break;
    case XOR_IMEDIATE_INS:
    case ADD_IM:
      fprintf(file, "%s %s %s %d\n", get_instruction(mips->instruction), registers[mips->destination], registers[mips->operandOne], mips->operandTwo);
      break;
    case BRANCH_EQ_INS:
    case BRANCH_NEQ_INS:
      fprintf(file, "%s %s %s label%d\n", get_instruction(mips->instruction), registers[mips->operandOne], registers[mips->operandTwo], mips->destination);
      break;
    case LABEL:
      fprintf(file, "label%d:\n", mips->operandOne);
      break;
    case JUMP:
      fprintf(file, "%s label%d\n", get_instruction(mips->instruction), mips->operandOne);
      break;
    case JUMPTOFUNC:
      fprintf(file, "%s function%d\n", get_instruction(mips->instruction), mips->operandOne);
      break;
    case FUNCTION_DEF:
      if (mips->operandOne == MAIN_FUNC)
      {
        fprintf(file, "main:\n");
        break;
      }
      fprintf(file, "function%d:\n", mips->operandOne);
      break;
    case SYSCALL:
      fprintf(file, "syscall\n");
      break;
    case LOADADDRESS:
      fprintf(file, "la %s function%d\n", registers[mips->destination], mips->operandOne);
      break;
    case JUMP_LINK_REG:
      fprintf(file, "jal %s\n", registers[mips->destination]);
      break;
  }

  if (mips->next) print_mips(mips->next, file);
}

void printTacBlock(TAC_BLOCK *block)
{
  TAC_BLOCK *pointer = block;
  while(pointer)
  {
    print_tac(pointer->tac);
    pointer = pointer->next;
  }
}
