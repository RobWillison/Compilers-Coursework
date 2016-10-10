

typedef struct UNION
{
  int           type;
  int           value;
  struct UNION  *pointer;

} UNION;

extern UNION* new_result(int, int);
