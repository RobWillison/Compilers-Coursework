#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "definitions.h"
#include "C.tab.h"
#include <string.h>
#include "TACstruct.h"
#include "MIPS.h"
#include "instructionSet.h"

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

  print_tac(tac_code->next);

  if (tac_code->operation == 'S') {
    LOCATION *destination = tac_code->destination;
    LOCATION *operand_one = tac_code->operand_one;
    printf("%s := %s\n", get_location(destination), get_location(operand_one));
  } else if (tac_code->operation == RETURN){
    LOCATION *operand_one = tac_code->operand_one;
    printf("RETURN %s\n", get_location(operand_one));
  } else if (tac_code->operation == IF_NOT){
    LOCATION *operand_one = tac_code->operand_one;
    LOCATION *operand_two = tac_code->operand_two;
    printf("IF NOT %s GOTO %d\n", get_location(operand_one), operand_two->value);
  } else if (tac_code->operation == IF){
    LOCATION *operand_one = tac_code->operand_one;
    LOCATION *operand_two = tac_code->operand_two;
    printf("IF %s GOTO %d\n", get_location(operand_one), operand_two->value);
  } else if (tac_code->operation == LABEL){
    printf("LABEL %d: ", tac_code->label);
  } else if (tac_code->operation == JUMP){
    LOCATION *operand_one = tac_code->operand_one;
    printf("GOTO %d\n", operand_one->value);
  } else if (tac_code->operation == FUNCTION_DEF){
    LOCATION *location = tac_code->operand_one;
    TOKEN *function = location->token;
    printf("%s:\n", function->lexeme);
  } else if (tac_code->operation == NEWFRAME){
    LOCATION *arguments = tac_code->destination;
    LOCATION *locals = tac_code->operand_one;
    LOCATION *tempories = tac_code->operand_two;

    printf("NEW FRAME %d arg %d loc %d temp\n", arguments->value, locals->value, tempories->value);
  } else {
    LOCATION *destination = tac_code->destination;
    LOCATION *operand_one = tac_code->operand_one;
    LOCATION *operand_two = tac_code->operand_two;

    printf("%s := %s %s %s\n", get_location(destination), get_location(operand_one), named(tac_code->operation), get_location(operand_two));
  }
}

void print_mips(MIPS *mips, FILE *file)
{
  if (!file)
  {
    file = stdout;
  }
  if (mips->next) print_mips(mips->next, file);

  switch (mips->instruction) {
    case LOADIMEDIATE_INS:
      fprintf(file, "%s %s %d\n", get_instruction(mips->instruction), registers[mips->destination], mips->operand_one);
      break;
    case STOREWORD_INS:
      fprintf(file, "%s %s %d($gp)\n", get_instruction(mips->instruction), registers[mips->operand_one], mips->destination);
      break;
    case LOADWORD_INS:
      fprintf(file, "%s %s %d($gp)\n", get_instruction(mips->instruction), registers[mips->destination], mips->operand_one);
      break;
    case '+':
    case '-':
    case SET_LESS_THAN_INS:
    case OR_INS:
      fprintf(file, "%s %s %s %s\n", get_instruction(mips->instruction), registers[mips->destination], registers[mips->operand_one], registers[mips->operand_two]);
      break;
    case '*':
    case '/':
      fprintf(file, "%s %s %s\n", get_instruction(mips->instruction), registers[mips->operand_one], registers[mips->operand_two]);
      break;
    case MOVE_LOW_INS:
      fprintf(file, "%s %s\n", get_instruction(mips->instruction), registers[mips->destination]);
      break;
    case XOR_IMEDIATE_INS:
      fprintf(file, "%s %s %s %d\n", get_instruction(mips->instruction), registers[mips->destination], registers[mips->operand_one], mips->operand_two);
      break;
    case BRANCH_EQ_INS:
    case BRANCH_NEQ_INS:
      fprintf(file, "%s %s %s label%d\n", get_instruction(mips->instruction), registers[mips->operand_one], registers[mips->operand_two], mips->destination);
      break;
    case LABEL:
      fprintf(file, "label%d:\n", mips->operand_one);
      break;
    case JUMP:
      fprintf(file, "%s label%d\n", get_instruction(mips->instruction), mips->operand_one);
      break;
    case FUNCTION_DEF:
      fprintf(file, "main:\n");
      break;
  }
}
