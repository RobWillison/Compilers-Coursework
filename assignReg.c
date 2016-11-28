#include <stdlib.h>
#include "C.tab.h"
#include "definitions.h"
#include "TACstruct.h"
#include "MIPS.h"
#include "instructionSet.h"
#include "assignReg.h"

int registerList[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
REG_USE *usedReg = NULL;

int findReg(int tempReg)
{
  int used = getAllocatedRegister(tempReg);
  if (used != -1) return used;
  return getFreeRegister(tempReg);
}

int getFreeRegister(int tempReg)
{

  REG_USE *newRegUse = (REG_USE*)malloc(sizeof(REG_USE));
  newRegUse->tempReg = tempReg;
  int reg = 0;
  if (!usedReg)
  {
    newRegUse->reg = registerList[reg];
    usedReg = newRegUse;
    return registerList[reg];
  }

  REG_USE *pointer = usedReg;
  while (pointer->next)
  {
    pointer = pointer->next;
    reg++;
  }

  newRegUse->reg = registerList[reg];
  pointer->next = newRegUse;
  return registerList[reg];
}

int getAllocatedRegister(int tempReg)
{
  if (!usedReg) return -1;

  REG_USE *pointer = usedReg;
  while (pointer)
  {
    if (pointer->tempReg == tempReg) return pointer->reg;
    pointer = pointer->next;
  }
  return -1;
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
      mips->destination = findReg(destination);
    }
    if (operandOne >= 100)
    {
      mips->operandOne = findReg(operandOne);
    }
    if (operandTwo >= 100)
    {
      mips->operandTwo = findReg(operandTwo);
    }

    mips = mips->next;
  }
}