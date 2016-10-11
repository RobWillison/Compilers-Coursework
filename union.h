

typedef struct UNION
{
  int           type;
  int           value;
  int           hasreturned;
  struct CLOSURE  *closure;
  struct UNION  *pointer;

} UNION;

extern UNION* new_result(int, int);
