#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "TACstruct.h"
#include "tacBlock.h"
#include "definitions.h"


void constantFolding(TAC_BLOCK *input)
{
  TAC *tac = input->tac;

  while (tac)
  {

  }
}

int isThisRegisterStoringAConstant(LOCATION *reg)
{
  
}

void optimiseTacBlock(TAC_BLOCK *input)
{
  constantFolding(input);
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
