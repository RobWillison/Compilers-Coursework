#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "TACstruct.h"
#include "tacBlock.h"

FUNCTION_BLOCK *functionLookupListHead = NULL;
FUNCTION_BLOCK *functionLookupListCurrent = NULL;
TAC_BLOCK *headBlock = NULL;
TAC_BLOCK *tailBlock = NULL;

void startGlobalBlock()
{
  addFunctionBlock(-1);
}

TAC_BLOCK *newBlockNotLinked()
{
  TAC_BLOCK *tacBlock = (TAC_BLOCK*)malloc(sizeof(TAC_BLOCK));
  tacBlock->tac = NULL;
  tacBlock->next = NULL;
  tacBlock->prev = NULL;

  return tacBlock;
}

void addFunctionBlock(int functionName)
{
  TAC_BLOCK *block = newBlockNotLinked();

  headBlock = block;
  tailBlock = block;

  FUNCTION_BLOCK *function = (FUNCTION_BLOCK*)malloc(sizeof(FUNCTION_BLOCK));
  function->block = block;
  function->function = functionName;

  if (!functionLookupListHead)
  {
    functionLookupListHead = function;
    functionLookupListCurrent = function;
    return;
  }

  FUNCTION_BLOCK *functionLookupListTail = functionLookupListHead;
  while (functionLookupListTail->next) functionLookupListTail = functionLookupListTail->next;

  function->prev = functionLookupListTail;
  functionLookupListTail->next = function;
  functionLookupListTail = function;
  functionLookupListCurrent = function;
}

void endFunctionBlock()
{
  functionLookupListCurrent = functionLookupListCurrent->prev;
  headBlock = functionLookupListCurrent->block;
  tailBlock = headBlock;

  while(tailBlock->next)
  {
    tailBlock = tailBlock->next;
  }
}

FUNCTION_BLOCK *getFunctionBlock(int function)
{
  FUNCTION_BLOCK *pointer = functionLookupListHead;
  while (pointer)
  {
    if (pointer->function == function) return pointer;
    pointer = pointer->next;
  }
}

TAC *new_tac()
{
  TAC *tac_struct = (TAC*)malloc(sizeof(TAC));
  tac_struct->destination = 0;
  tac_struct->next = 0;
  tac_struct->label = 0;

  return tac_struct;
}

TAC *newTac()
{
  TAC *tac_struct = new_tac();

  if (!tailBlock->tac)
  {
    tailBlock->tac = tac_struct;
    return tac_struct;
  }

  TAC *pointer = tailBlock->tac;

  while (pointer->next) pointer = pointer->next;

  pointer->next = tac_struct;

  return tac_struct;
}

TAC_BLOCK *newBlock()
{
  TAC_BLOCK *tacBlock = (TAC_BLOCK*)malloc(sizeof(TAC_BLOCK));
  tacBlock->tac = NULL;
  tacBlock->next = NULL;
  tacBlock->prev = NULL;

  if (!headBlock)
  {
    tailBlock = tacBlock;
    headBlock = tacBlock;

    return headBlock;
  }

  tacBlock->prev = tailBlock;
  tailBlock->next = tacBlock;
  tailBlock = tacBlock;

  return tacBlock;
}

TAC_BLOCK *getHeadTacBlock()
{
  FUNCTION_BLOCK *functionHead = functionLookupListHead;
  TAC_BLOCK *blockList = functionHead->block;
  TAC_BLOCK *blockListPointer = blockList;
  functionHead = functionHead->next;

  while (functionHead)
  {
    while (blockListPointer->next) blockListPointer = blockListPointer->next;
    blockListPointer->next = functionHead->block;

    functionHead = functionHead->next;
  }

  return blockList;
}

TAC *getLastInstruction()
{
  TAC *pointer = tailBlock->tac;
  while (pointer->next)
  {
    pointer = pointer->next;
  }
  return pointer;
}

LOCATION *getLastInstructionDestination()
{
  TAC *lastIns = getLastInstruction();

  return lastIns->destination;
}

int getLastInstructionOperation()
{
  TAC *lastIns = getLastInstruction();

  return lastIns->operation;
}

int countTemporiesInBlock(FUNCTION_BLOCK *funcBlock)
{
  int count = 0;
  TAC_BLOCK *block = funcBlock->block;
  while (block)
  {
    TAC *tac_code = block->tac;
    while (tac_code)
    {
      LOCATION *destination = tac_code->destination;

      if ((destination) && (destination->type == LOCREG)) count += 1;
      tac_code = tac_code->next;
    }
    block = block->next;
  }

  return count;
}

int countLocalsInBlock(FUNCTION_BLOCK *funcBlock)
{
  int count = 0;
  TAC_BLOCK *block = funcBlock->block;
  while (block)
  {
    TAC *tac_code = block->tac;
    while (tac_code)
    {
      LOCATION *destination = tac_code->destination;

      if ((destination) && (destination->type == LOCTOKEN)) count += 1;
      tac_code = tac_code->next;
    }
    block = block->next;
  }

  return count;
}
