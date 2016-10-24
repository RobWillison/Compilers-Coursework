
extern void print_variable_list(VARIABLE *pointer);

extern void print_enviroment(FRAME *enviroment);

extern char *named(int t);

extern void print_leaf(NODE *tree, int level);

extern void print_tree0(NODE *tree, int level);

extern void print_tree(NODE *tree);

extern void print_tac(TAC *tac);

extern void print_mips(MIPS *mips, FILE *file);
