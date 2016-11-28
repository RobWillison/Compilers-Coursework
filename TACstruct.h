#define LOCTOKEN 5000
#define LOCREG 4000
#define LOCVALUE 6000
#define LOCCLOSURE 7000

#define LABEL 3000
#define FUNCTION_DEF 6000
#define NEWFRAME 7000
#define LOADPARAM 8000
#define CREATE_CLOSURE 9000

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
  struct LOCATION *operandOne;
  struct LOCATION *operandTwo;
  struct TAC  *next;
} TAC;

extern TAC* newTac();
