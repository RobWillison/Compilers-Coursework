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
#include "instructionSet.h"

void removeTacFromBlock(TAC *tac, TAC_BLOCK *block)
{
  TAC *pointer = block->tac;

  if (pointer == tac)
  {
    block->tac = pointer->next;
    computeNextUseInfo(block);
    return;
  }

  TAC *prev = pointer;
  pointer = pointer->next;

  while (pointer)
  {
    if (pointer == tac)
    {
      prev->next = pointer->next;
      computeNextUseInfo(block);
      return;
    }
    prev = pointer;
    pointer = pointer->next;
  }



}

int compareLocation(LOCATION *one, LOCATION *two)
{
  if (!one || !two) return 0;

  return (one->type == LOCREG && two->type == LOCREG && one->reg == two->reg)
        || (one->type == LOCTOKEN && two->type == LOCTOKEN && one->token == two->token)
        || (one->type == LOCVALUE && two->type == LOCVALUE && one->value == two->value)
        || (one->type == LOCCLOSURE && two->type == LOCCLOSURE && one->value == two->value);
}

int constantFolding(TAC *tac)
{

  LOCATION *operandOne = tac->operand_one;
  LOCATION *operandTwo = tac->operand_two;

  if (!(operandOne && operandTwo)) return 0;

  TAC *operandOneDef = getDefinitionTac(tac, operandOne);
  TAC *operandTwoDef = getDefinitionTac(tac, operandTwo);

  if (!operandOneDef) return 0;
  if (!operandTwoDef) return 0;

  if (!(operandOneDef->operation == 'S' && operandTwoDef->operation == 'S')) return 0;

  LOCATION *operandOneLoc = operandOneDef->operand_one;
  LOCATION *operandTwoLoc = operandTwoDef->operand_one;

  if (operandOneLoc->type != LOCTOKEN || operandTwoLoc->type != LOCTOKEN) return 0;

  TOKEN *operandOneTok = operandOneLoc->token;
  TOKEN *operandTwoTok = operandTwoLoc->token;

  if (operandOneTok->type != CONSTANT || operandTwoTok->type != CONSTANT) return 0;

  LOCATION *newLocation = new_location(LOCVALUE);
  tac->operand_one = newLocation;

  switch (tac->operation) {
    case '+':
      newLocation->value = operandOneTok->value + operandTwoTok->value;
      break;
    case '-':
      newLocation->value = operandOneTok->value - operandTwoTok->value;
      break;
    case '/':
      newLocation->value = operandOneTok->value / operandTwoTok->value;
      break;
    case '*':
      newLocation->value = operandOneTok->value * operandTwoTok->value;
      break;
  }

  tac->operation = 'S';
  tac->operand_two = 0;

  return 1;
}

int copyProporgation(TAC *tac)
{
  int changed = 0;
  if (tac->operation != 'S') return 0;

  if (tac->operand_two && ((LOCATION*)tac->operand_two)->value != 0) return 0;

  TAC *pointer = tac;
  pointer = pointer->next;
  if (!pointer) return 0;

  while (!compareLocation(pointer->destination, tac->operand_one)
            && !(pointer->operation == PARAMETER_ALLOCATE && ((LOCATION*)tac->operand_one)->reg == RETURN_REG))
  {
    if (compareLocation(pointer->operand_one, tac->destination))
    {
      pointer->operand_one = tac->operand_one;
      changed++;
    }

    if (compareLocation(pointer->operand_two, tac->destination))
    {
      pointer->operand_two = tac->operand_one;
      changed++;
    }


    pointer = pointer->next;

    if (pointer) continue;
    return changed;
  }

  return 0;
}

int deadCodeElimination(TAC *tac, TAC_BLOCK *block)
{
  switch (tac->operation)
  {
    case 'S':
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
      break;
    default:
      return 0;
  }

  VARIABLE_NEXT_USE *nextUseInfo = getVariableNextUse(tac->destination);

  while (nextUseInfo)
  {
    if (nextUseInfo->nextUse == tac)
    {
      if (nextUseInfo->live) return 0;
      if (!nextUseInfo->prev)
      {
        removeTacFromBlock(tac, block);
        return 1;
      }
      if (nextUseInfo->prev->live == 0)
      {
        removeTacFromBlock(tac, block);
        return 1;
      }
    }

    nextUseInfo = nextUseInfo->next;
  }

  return 0;

}

int commonSubExpression(TAC *tac)
{

}


int optimiseTacOperation(TAC *tac, TAC_BLOCK *block)
{
  int changed = 0;
  if (tac->next) changed = optimiseTacOperation(tac->next, block);

  changed = changed + constantFolding(tac);
  changed = changed + copyProporgation(tac);
  changed = changed + deadCodeElimination(tac, block);

  return changed;
}

void optimiseTacBlock(TAC_BLOCK *input)
{
  clearNextUseInfo();
  NEXT_USE_INFO *nextUseInfo = computeNextUseInfo(input);

  while(optimiseTacOperation(input->tac, input))
  {
    clearNextUseInfo();
    NEXT_USE_INFO *nextUseInfo = computeNextUseInfo(input);
  }
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
