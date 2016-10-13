#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "debug.h"
#include "TACstruct.h"

int current_reg = 0;

#define LOCTOKEN 5
#define LOCREG 4

extern TAC *compile(NODE *tree);

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
  while (tac_code != 0) {
    if (tac_code->operation = 'S') {
      LOCATION *destination = tac_code->destination;
      LOCATION *operand_one = tac_code->operand_one;

      printf("%s := %s\n", get_location(destination), get_location(operand_one));
    }
    tac_code = tac_code->next;
  }

}

TAC *new_tac(int destination)
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

LOCATION *next_reg()
{
  current_reg += 1;
  LOCATION *loc = new_location(LOCREG);
  loc->reg = current_reg;
  return loc;
}

TAC *compile(NODE *tree)
{
  printf("NEXT TREE\n");
  print_tree(tree);

  if (tree->type == RETURN)
  {
    TAC *operand_tac = compile(tree->left);
    operand_tac->next = new_tac(next_reg());
    operand_tac->next->operation = 'S';
    operand_tac->next->operand_one = operand_tac->destination;

    return operand_tac;
  } else if (tree->type == LEAF) {
      if(tree->left->type == CONSTANT) {
        TOKEN *t = (TOKEN *)tree->left;
        LOCATION *loc = new_location(LOCTOKEN);
        loc->token = t;

        TAC *taccode = new_tac(next_reg());
        taccode->operation = 'S';
        taccode->operand_one = loc;

        return taccode;
      }
  }
}
