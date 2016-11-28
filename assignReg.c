#include <stdlib.h>
#include "C.tab.h"
#include "definitions.h"
#include "TACstruct.h"
#include "MIPS.h"
#include "instructionSet.h"
#include "assignReg.h"

int registerList[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};

SPACE *getAllocatedSpace(int tempReg, MIPS_BLOCK *block)
{
  if (!block->space) return NULL;

  SPACE *pointer = block->space;
  while (pointer)
  {
    if (pointer->tempReg == tempReg) return pointer;
    pointer = pointer->next;
  }
  return NULL;
}

SPACE *moveToMemory(int temp, MIPS_BLOCK *block)
{
  SPACE *space = getAllocatedSpace(temp);
  space->type == MEMORY;
  space->memory_location = getFreeMemoy();
}

SPACE *getFreeRegister(int tempReg, MIPS_BLOCK *block)
{

  SPACE *newRegUse = (SPACE*)malloc(sizeof(SPACE));
  newRegUse->tempReg = tempReg;
  newRegUse->type = REGISTER;
  newRegUse->next = NULL;
  int reg = 0;
  if (!block->space)
  {
    newRegUse->reg = registerList[reg];
    block->space = newRegUse;
    return newRegUse;
  }

  SPACE *pointer = block->space;
  while (pointer->next)
  {
    pointer = pointer->next;
    reg++;
  }

  newRegUse->reg = registerList[reg];
  pointer->next = newRegUse;
  return newRegUse;
}

int getFreeMemory(MIPS_BLOCK *block)
{
  int memory = 12;
  if (!block->space) return memory;

  SPACE *pointer = block->space;
  while (pointer->next)
  {
    pointer = pointer->next;
    memory = memory + 4;
  }

  return memory;
}

SPACE *findSpace(int tempReg, MIPS_BLOCK *block)
{
  SPACE *used = getAllocatedSpace(tempReg, block);
  if (used) return used;
  return getFreeRegister(tempReg, block);
}

void endBlockSave(MIPS_BLOCK *block)
{
  SPACE *space = block->space;
  SPACE *start = space;
  while (space->next) space = space->next;

  SPACE *end = space;

  int cont = 1;
  while (cont)
  {
    SPACE *newSpace = getFreeMemory(start->tempReg, block);
    create_mips_instruction(STOREWORD, 30, newSpace->memory_location, newSpace->reg);
    start = start->next;
    if (start == end) cont = 0;
  }
}

void assignBlockReg(MIPS_BLOCK *block)
{
  MIPS *mips = block->instructions;
  while (mips)
  {
    int destination = mips->destination;
    int operandOne = mips->operandOne;
    int operandTwo = mips->operandTwo;

    if (destination >= 100)
    {
      SPACE *space = findSpace(destination, block);
      mips->destination = space->reg;
    }
    if (operandOne >= 100)
    {
      SPACE *space = findSpace(operandOne, block);
      mips->operandOne = space->reg;
    }
    if (operandTwo >= 100)
    {
      SPACE *space = findSpace(operandTwo, block);
      mips->operandTwo = space->reg;
    }

    mips = mips->next;
  }

  endBlockSave(block);
}