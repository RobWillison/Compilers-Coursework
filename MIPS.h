#define JUMP 300
#define JUMPTOFUNC 301
#define JUMPTOADDRS 302

typedef struct MIPS
{
  int instruction;
  int destination;
  int operand_one;
  int operand_two;
  struct MIPS *next;
} MIPS;

extern const char* registers[];

extern MIPS* new_mips();

extern char *get_instruction(int instruction);
