#include "definitions.h"

void print_variable_list(VARIABLE *pointer);

void print_enviroment(FRAME *enviroment);

char *named(int t);

void print_leaf(NODE *tree, int level);

void print_tree0(NODE *tree, int level);

void print_tree(NODE *tree);
