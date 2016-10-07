/*
 * Adapted from
 * CM20029 Coursework Assignment 1
 * Tom Crick
 * cs1tc@bath.ac.uk
 * 30 Apr 2003
 *
 * symbol_table.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "C.tab.h"



#define HASH_SIZE (1000)
#define LIST 898;

TOKEN *int_token, *void_token, *function_token;

TOKEN** init_symbtable(void)
{
    TOKEN **symbtable = (TOKEN**)calloc(HASH_SIZE, sizeof(TOKEN*));

    int_token = new_token(INT);
    int_token->lexeme = "int";
    function_token = new_token(FUNCTION);
    function_token->lexeme = "function";
    void_token = new_token(VOID);
    void_token->lexeme = "void";

    return symbtable;
}

int hash(char *s)
{
    int h = 0;
    while (*s != '\0') {
      h = (h<<4) ^ *s++;
    }
    return (0x7fffffff&h) % HASH_SIZE;
}

TOKEN* lookup_token(TOKEN **symbtable, char *s)
{
    int	h = hash(s);
    TOKEN *a = symbtable[h];
    TOKEN *ans;
/*     printf("\nLookup: %s\n", s); */
    while (a!=NULL) {
      if (strcmp(a->lexeme, s)==0) return a;
      a = a->next;
    }
    ans = new_token(IDENTIFIER);
    ans->lexeme = (char*)malloc(1+strlen(s));
    strcpy(ans->lexeme, s);
    ans->next = symbtable[h];
    symbtable[h] = ans;
/*     printf(" stored at %p\n", ans); */

    return ans;
}

void add_token(TOKEN **symbtable, TOKEN *token)
{
  int h = hash(token->lexeme);

  if (symbtable[h] == NULL) {
    symbtable[h] = token;
  } else {
    symbtable[h]->next = token;
  }
}
