#define INMEMORY 1


extern MIPS *translate_tac(TAC *tac);

typedef struct MIPS_FRAME
{
  struct MIPS_BINDING  *bindings;
  struct MIPS_FRAME  *prev;
} MIPS_FRAME;


typedef struct MIPS_CLOSURE
{
  char *name;
  struct MIPS_LOCATION *enclosing_frame;
} MIPS_CLOSURE;

typedef struct MIPS_LOCATION
{
  int type;
  int reg;
  int memory_frame_location;
} MIPS_LOCATION;

typedef struct MIPS_STORED_VALUE
{
  int type;
  struct MIPS_CLOSURE *closure;
  struct MIPS_LOCATION *location;
} MIPS_STORED_VALUE;

typedef struct MIPS_BINDING
{
  int type;
  LOCATION *tac_location;
  struct MIPS_STORED_VALUE *value;
  struct MIPS_BINDING *next;
} MIPS_BINDING;
