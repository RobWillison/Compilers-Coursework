#define INMEMORY 1
#define INREG 2

extern MIPS *translate_tac(TAC_BLOCK *tac);

typedef struct MIPS_FRAME
{
  struct MIPS_BINDING  *bindings;
  struct MIPS_FRAME  *prev;
} MIPS_FRAME;

typedef struct MIPS_LOCATION
{
  int type;
  int reg;
  int memory_frame_location;
} MIPS_LOCATION;

typedef struct MIPS_BINDING
{
  int type;
  LOCATION *tac_location;
  struct MIPS_LOCATION *location;
  struct MIPS_BINDING *next;
} MIPS_BINDING;
