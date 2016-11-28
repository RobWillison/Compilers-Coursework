typedef struct REG_USE
{
  int tempReg;
  int reg;
  struct REG_USE *next;
} REG_USE;


extern void assignBlockReg(MIPS_BLOCK *block);