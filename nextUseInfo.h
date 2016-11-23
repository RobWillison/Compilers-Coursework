typedef struct NEXT_USE_INFO
{
  struct VARIABLE_NEXT_USE *variable;
  struct NEXT_USE_INFO *next;
} NEXT_USE_INFO;

typedef struct VARIABLE_NEXT_USE
{
  struct LOCATION *location;
  struct TAC *nextUse;
  int live;
  struct VARIABLE_NEXT_USE *next;
} VARIABLE_NEXT_USE;

extern NEXT_USE_INFO *getNextUseInfo(TAC_BLOCK *input);
extern TAC *getDefinitionTac(TAC *instuction, LOCATION *variable);
extern void clearNextUseInfo();
