#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "C.tab.h"
#include "TACstruct.h"
#include "MIPS.h"
#include "definitions.h"
#include "tac_compiler.h"
#include "optimiser.h"
#include "tacBlock.h"
#include "debug.h"
#include "interpreter.h"

#define ANSWERVALUE 254

extern int yydebug;
extern NODE* yyparse(char* fileName);
extern NODE* ans;
extern TOKEN** init_symbtable(void);

int main(int argc, char** argv)
{
    char* fileName = "";
    NODE* tree;
    int compileset = 0;

    if (strcmp(argv[1], "-c") == 0) {
      compileset = 1;
    }

    fileName = argv[2];

    if (argc>3 && strcmp(argv[3],"-d")==0) yydebug = 1;

    init_symbtable();
    printf("--C COMPILER\n");
    yyparse(fileName);
    tree = ans;
    printf("parse finished with %p\n", tree);
    print_tree(tree);

    if (!compileset)
    {
      printf("Starting Interpretation\n");

      FRAME *enviroment = new_frame();
      UNION *result = (UNION*)intepret(tree, enviroment);

      printf("\nRESULT: %d\n", result->value);

      return result->value;
    } else {
      TAC_BLOCK *taccode = compile(tree);
      printf("COMPILED TO TAC\n");
      printTacBlock(taccode);
      taccode = optimiseTac(taccode);
      printf("TAC OPTIMISED\n");
      printTacBlock(taccode);
      printf("TRANSLATING TO MIPS\n");
      MIPS *ins = translate_tac(taccode);
      printf("TRANSLATED TO MIPS\n");
      ins = optimise(ins);
      FILE *file = fopen("Output/test.asm", "w");
      fprintf(file, ".globl main\n\n.text\n\n");
      print_mips(ins, file);
      print_mips(ins, 0);
      fprintf(file, "\n.data\n");
    }
}
