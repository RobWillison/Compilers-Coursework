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

void print_tac_single(TAC *tac)
{
  if (!tac) return;
  TAC *temp = tac->next;
  tac->next = NULL;
  print_tac(tac);
  tac->next = temp;
}

void printNextUse()
{
  NEXT_USE_INFO *pointer = nextUseInfoList;
  while (pointer) {
    VARIABLE_NEXT_USE *var = pointer->variable;
    printf("VARIBALE %s\n", get_location(var->location));
    while (var)
    {
      printf("Live: %d\n", var->live);
      print_tac_single(var->nextUse);
      var = var->next;
    }
    pointer = pointer->next;
  }
}

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

void addNextUseInfo(LOCATION *location, int live, TAC *nextUse)
{
  //Check the location isnt a constant value
  if (location->type == LOCVALUE || (location->type == LOCTOKEN && ((TOKEN*)location->token)->type == CONSTANT)) return;


  VARIABLE_NEXT_USE *variable = (VARIABLE_NEXT_USE*)malloc(sizeof(VARIABLE_NEXT_USE));
  variable->live = live;
  variable->nextUse = nextUse;
  variable->location = location;

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

NEXT_USE_INFO *getNextUseInfoTac(TAC *tac)
{
  if (tac->next) getNextUseInfoTac(tac->next);

  switch (tac->operation) {
    case 'S':
      addNextUseInfo(tac->destination, 0, tac);
      addNextUseInfo(tac->operand_one, 1, tac);
      break;
    case '+':
    case '-':
    case '*':
    case '/':
      addNextUseInfo(tac->destination, 0, tac);
      addNextUseInfo(tac->operand_one, 1, tac);
      addNextUseInfo(tac->operand_two, 1, tac);
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

NEXT_USE_INFO *getNextUseInfo(TAC_BLOCK *input)
{
  NEXT_USE_INFO *temp = getNextUseInfoTac(input->tac);
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
