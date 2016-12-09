
typedef struct TAC_BLOCK {
  struct TAC *tac;
  struct TAC_BLOCK *next;
  struct TAC_BLOCK *prev;
} TAC_BLOCK;

typedef struct FUNCTION_BLOCK {
  struct TAC_BLOCK *block;
  int function;
  struct FUNCTION_BLOCK *next;
  struct FUNCTION_BLOCK *prev;
} FUNCTION_BLOCK;

extern void addFunctionBlock(int functionName);
extern FUNCTION_BLOCK *getFunctionBlock(int functionName);
extern TAC_BLOCK *newFunctionBlock();
extern TAC *newTac();
extern TAC_BLOCK *newBlock();
extern TAC_BLOCK *getHeadTacBlock();
extern LOCATION *getLastInstructionDestination();
extern int getLastInstructionOperation();
extern void startGlobalBlock();
extern int countLocalsInBlock(FUNCTION_BLOCK *block);
extern int countTemporiesInBlock(FUNCTION_BLOCK *block);
extern void endFunctionBlock();
