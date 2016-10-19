typedef struct LOCATION
{
  int   type;
  struct TOKEN *token;
  int reg;
  int value;
} LOCATION;

extern LOCATION* new_location(int type);

typedef struct TAC
{
  struct LOCATION *destination;
  int    operation;
  int    label;
  struct LOCATION *operand_one;
  struct LOCATION *operand_two;
  struct TAC  *next;
} TAC;

extern TAC* new_tac();
