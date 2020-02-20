/* Juan Carlos Díaz Martín. Universidad de Extremadura 
   3 de febrero 2010
   Pila ordinaria con cotador de items apilados
 */ 
#include <stack.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 

//#define DEBUG  1

void STACK_init(STACK_t stack)
{
#ifdef DEBUG
  printf("  STACK_create: Entro \n");
#endif
  stack->Top     = NULL;
#ifdef DEBUG
  printf("  STACK_create: Salgo \n");
#endif
  return;
}

/*
void STACK_push(STACK_t stack, STACK_link_t item)
{
#ifdef DEBUG
  printf("  STACK_push: Entro \n");
#endif

  item->Link = stack->Top;
  stack->Top = item;

#ifdef DEBUG
  printf("  STACK_push: Salgo \n");
#endif
  return;
}
*/

int STACK_pop(STACK_t stack, STACK_link_t *item)
{
#ifdef DEBUG
  printf("  STACK_pop: Entro \n");
#endif

  if(stack->Top == NULL)                                                        goto exception_empty;
  *item = stack->Top;
  stack->Top = stack->Top->Link;

#ifdef DEBUG
  printf("  STACK_pop: Salgo \n"); 
#endif
  return 0;

exception_empty:
#ifdef DEBUG
  printf("  STACK_pop: Empty stack exception \n");
#endif
  return -1;
}

