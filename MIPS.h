#define JUMP 300

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
extern char *get_location(LOCATION *location);
