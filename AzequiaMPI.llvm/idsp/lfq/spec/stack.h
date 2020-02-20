#ifndef _STACK_H_
#define _STACK_H_

typedef struct STACK      *STACK_t;
typedef struct STACK_link *STACK_link_t;

struct STACK_link {
  STACK_link_t  Link; 
};
typedef struct STACK_link STACK_link;

struct STACK {
  STACK_link_t  Top; 
};
typedef struct STACK STACK;

void STACK_init    (STACK_t  stack);
void STACK_push    (STACK_t  stack, STACK_link_t  link);
int  STACK_pop     (STACK_t  stack, STACK_link_t *link);



#define STACK_push(stack, item) \
{ \
  (item)->Link = (stack)->Top; \
  (stack)->Top = (item);       \
}

#endif
