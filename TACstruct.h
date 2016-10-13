typedef struct LOCATION
{
  int   type;
  struct TOKEN *token;
  int reg;
} LOCATION;

extern LOCATION* new_location(int type);

typedef struct TAC
{
  struct LOCATION *destination;
  int   operation;
  struct LOCATION *operand_one;
  struct LOCATION *operand_two;
  struct TAC  *next;
} TAC;

extern TAC* new_tac(int destination);
