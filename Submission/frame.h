

typedef struct FRAME
{
  struct VARIABLE  *value;
  struct FRAME  *next;
} FRAME;

extern FRAME* new_frame();

typedef struct VARIABLE
{
  struct TOKEN     *token;
  struct UNION     *value;
  struct VARIABLE  *next;
} VARIABLE;

extern VARIABLE* new_variable(TOKEN *, UNION *);
