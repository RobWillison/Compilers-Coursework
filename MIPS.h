#define JUMP 300
#define JUMPTOFUNC 301
#define PARAMETER_ALLOCATE 303
#define SAVE_PARAM 304
#define FUNC_END 305

typedef struct MIPS
{
  int instruction;
  int destination;
  int operand_one;
  int operand_two;
  struct MIPS *next;
} MIPS;

typedef struct MIPS_BLOCK
{
  struct MIPS *instructions;
  struct MIPS_BLOCK *next;
} MIPS_BLOCK;

extern const char* registers[];

extern MIPS* new_mips();

extern char *get_instruction(int instruction);
extern MIPS *create_mips_instruction(int x, int y, int z, int a);
extern MIPS *create_load_ins(LOCATION *destination, LOCATION *operand);
extern MIPS *getProgramHead();
extern void newMIPSBlock();
extern void add_MIPS_to_list(MIPS *head, MIPS *tail);
