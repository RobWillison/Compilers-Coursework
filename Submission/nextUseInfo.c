#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "TACstruct.h"
#include "tacBlock.h"
#include "definitions.h"
#include "nextUseInfo.h"
#include "MIPS.h"

NEXT_USE_INFO *nextUseInfoList = NULL;

int locationIsALocal(LOCATION *location)
{
  if (location->type != LOCTOKEN) return 0;
  TOKEN *token = location->token;

  return token->type == IDENTIFIER;
}

NEXT_USE_INFO *newNextUseInfo(VARIABLE_NEXT_USE *variable)
{
  NEXT_USE_INFO *nextUse = (NEXT_USE_INFO*)malloc(sizeof(NEXT_USE_INFO));


  if (locationIsALocal(variable->location))
  {
    VARIABLE_NEXT_USE *varNextUse = (VARIABLE_NEXT_USE*)malloc(sizeof(VARIABLE_NEXT_USE));
    varNextUse->live = 1;
    varNextUse->nextUse = NULL;
    varNextUse->location = variable->location;

    nextUse->variable = varNextUse;
    varNextUse->next = variable;
    variable->prev = varNextUse;

    return nextUse;
  }

  nextUse->variable = variable;

  return nextUse;
}

void addToEndOfNextUseList(VARIABLE_NEXT_USE *variable, VARIABLE_NEXT_USE *list)
{
  VARIABLE_NEXT_USE *pointer = list;
  while (pointer->next)
  {
    pointer = pointer->next;
  }

  variable->prev = pointer;
  pointer->next = variable;
}

void addNextUseInfo(LOCATION *location, int live, TAC *nextUse)
{
  //Check the location isnt a constant value
  if (location->type == LOCVALUE || (location->type == LOCTOKEN && ((TOKEN*)location->token)->type == CONSTANT)) return;


  VARIABLE_NEXT_USE *variable = (VARIABLE_NEXT_USE*)malloc(sizeof(VARIABLE_NEXT_USE));
  variable->live = live;
  variable->nextUse = nextUse;
  variable->location = location;
  variable->prev = 0;

  if (!nextUseInfoList)
  {
    nextUseInfoList = newNextUseInfo(variable);
    return;
  }

  NEXT_USE_INFO *pointer = nextUseInfoList;
  while(pointer)
  {
    LOCATION *pointerLocation = ((VARIABLE_NEXT_USE*)pointer->variable)->location;

    if (pointerLocation->type == LOCREG && pointerLocation->reg == location->reg
          || pointerLocation->type == LOCTOKEN && pointerLocation->token == location->token)
    {
      addToEndOfNextUseList(variable, pointer->variable);
      return;
    }
    if (!pointer->next) break;

    pointer = pointer->next;
  }

  pointer->next = newNextUseInfo(variable);
}

NEXT_USE_INFO *computeNextUseInfoTac(TAC *tac)
{
  if (tac->next) computeNextUseInfoTac(tac->next);

  switch (tac->operation) {
    case 'S':
      addNextUseInfo(tac->destination, 0, tac);
      addNextUseInfo(tac->operandOne, 1, tac);
      break;
    case '+':
    case '-':
    case '*':
    case '/':
      addNextUseInfo(tac->destination, 0, tac);
      addNextUseInfo(tac->operandOne, 1, tac);
      addNextUseInfo(tac->operandTwo, 1, tac);
    case RETURN:
      addNextUseInfo(tac->operandOne, 1, tac);
    case JUMPTOFUNC:
      addNextUseInfo(tac->operandOne, 1, tac);
    case SAVE_PARAM:
      addNextUseInfo(tac->operandOne, 1, tac);
    case IF_NOT:
      addNextUseInfo(tac->operandOne, 1, tac);
  }
}

VARIABLE_NEXT_USE *getVariableNextUse(LOCATION *target)
{
  NEXT_USE_INFO *pointer = nextUseInfoList;
  while (pointer)
  {
    VARIABLE_NEXT_USE *variable = pointer->variable;
    if (variable->location == target) return variable;
    pointer = pointer->next;
  }
}

NEXT_USE_INFO *computeNextUseInfo(TAC_BLOCK *input)
{
  NEXT_USE_INFO *temp = computeNextUseInfoTac(input->tac);
  return temp;
}

TAC *getDefinitionTac(TAC *instuction, LOCATION *variable)
{
  VARIABLE_NEXT_USE *nextUseInfo = getVariableNextUse(variable);

  int foundMyInstruction = 0;

  while (nextUseInfo)
  {
    if (nextUseInfo->nextUse == instuction)
    {
      foundMyInstruction = 1;
    }
    if (foundMyInstruction)
    {
      if (nextUseInfo->live == 0)
      {
        return nextUseInfo->nextUse;
      }
    }

    nextUseInfo = nextUseInfo->next;
  }
}

void clearNextUseInfo()
{
  nextUseInfoList = NULL;
}
