#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "TACstruct.h"
#include "tacBlock.h"
#include "definitions.h"

typedef struct NEXT_USE_INFO
{
  struct VARIABLE_NEXT_USE *variable;
  struct NEXT_USE_INFO *next;
} NEXT_USE_INFO;

typedef struct VARIABLE_NEXT_USE
{
  struct LOCATION *location;
  struct TAC *nextUse;
  int live;
  struct VARIABLE_NEXT_USE *next;
} VARIABLE_NEXT_USE;

NEXT_USE_INFO *nextUseInfoList = NULL;

NEXT_USE_INFO *newNextUseInfo(VARIABLE_NEXT_USE *variable)
{
  NEXT_USE_INFO *nextUse = (NEXT_USE_INFO*)malloc(sizeof(NEXT_USE_INFO));
  nextUse->variable = variable;

  return nextUse;
}

void addToEndOfNextUseList(VARIABLE_NEXT_USE *variable, VARIABLE_NEXT_USE *list)
{
  VARIABLE_NEXT_USE *pointer = list;
  while (pointer->next) pointer = pointer->next;
  pointer->next = variable;
}

VARIABLE_NEXT_USE *addNextUseInfo(LOCATION *location, int live, TAC *nextUse)
{
  VARIABLE_NEXT_USE *variable = (VARIABLE_NEXT_USE*)malloc(sizeof(VARIABLE_NEXT_USE));
  variable->live = live;
  variable->nextUse = nextUse;

  if (!nextUseInfoList)
  {
    nextUseInfoList = newNextUseInfo(variable);
    return;
  }

  NEXT_USE_INFO *pointer = nextUseInfoList;
  while(pointer)
  {
    if (((VARIABLE_NEXT_USE*)pointer->variable)->location == location)
    {
      addToEndOfNextUseList(variable, pointer->variable);
      return;
    }

    if (!pointer->next) break;

    pointer = pointer->next;
  }

  pointer->next = newNextUseInfo(variable);
}

NEXT_USE_INFO *getNextUseInfoTac(TAC *tac)
{
  if (tac->next) getNextUseInfoTac(tac->next);

  LOCATION *destination = tac->destination;
  LOCATION *operandOne = tac->operand_one;
  LOCATION *operandTwo = tac->operand_two;

  if (destination)
  {
    addNextUseInfo(destination, 0, NULL);
  }
  if (operandOne)
  {
    addNextUseInfo(operandOne, 1, tac);
  }
  if (operandTwo)
  {
    addNextUseInfo(operandTwo, 1, tac);
  }

  return nextUseInfoList;
}

NEXT_USE_INFO *getNextUseInfo(TAC_BLOCK *input)
{
  return getNextUseInfoTac(input->tac);
}

void optimiseTacBlock(TAC_BLOCK *input)
{
  getNextUseInfo(input);
}

TAC_BLOCK *optimiseTac(TAC_BLOCK *input)
{
  TAC_BLOCK *pointer = input;
  while(pointer)
  {
    optimiseTacBlock(pointer);
    pointer = pointer->next;
  }

  return input;
}
