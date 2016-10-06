

typedef struct RESULT
{
  int           terminated;
  int           value;

} RESULT;

extern RESULT* new_result(int value);
