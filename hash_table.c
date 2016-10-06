#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "C.tab.h"

#define HASH_SIZE (1000)
#define LIST 898;

extern hash(char *s);


TOKEN** init_hashtable(void)
{
    TOKEN **hashtable = (TOKEN**)calloc(HASH_SIZE, sizeof(TOKEN*));

    return hashtable;
}


TOKEN* lookup_variable(TOKEN **hashtable, char *s)
{
    printf("LOOKING FOR %s:", s);
    int	h = hash(s);
    TOKEN *a = hashtable[h];
    TOKEN *ans;
/*     printf("\nLookup: %s\n", s); */
    while (a!=NULL) {
      if (strcmp(a->lexeme, s)==0) return a;
      a = a->next;
    }
    ans = new_token(IDENTIFIER);
    ans->lexeme = (char*)malloc(1+strlen(s));
    strcpy(ans->lexeme, s);
    ans->next = hashtable[h];
    hashtable[h] = ans;
/*     printf(" stored at %p\n", ans); */

    return ans;
}

void add_variable(TOKEN **hashtable, TOKEN *token)
{

  int h = hash(token->lexeme);

  if (hashtable[h] == NULL) {
    hashtable[h] = token;
  } else {
    hashtable[h]->next = token;
  }
}
