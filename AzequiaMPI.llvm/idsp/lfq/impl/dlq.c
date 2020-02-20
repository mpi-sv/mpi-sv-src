#include <addr.h>
#include <stdio.h>
#include <dlq.h>

//#define NULL 0



/*
void DLQ_find(DLQ_t dlq, Link_t *item, Addr_t addr, Tag_t tag, int type, int mode, int *success)
{
  register Link_t lprev = &(dlq->Queue);
  register Link_t link  = dlq->Queue.Next;
                                                               
  *success = FALSE; 
  *item = NULL;                                    
  if (!dlq->cmp) {                                           
  //fprintf(stdout, "DLQ_find(%p): Cola sin criterio. No encontrada\n", (void *)THR_self()); fflush(stdout);
    *success = FALSE;                                 
    return;                                                     
  }                                                            
  if (DLQ_isEmpty(dlq)) {                                      
  //fprintf(stdout, "DLQ_find(%p): Cola vacía. No encontrada\n", (void *)THR_self()); fflush(stdout);       
    *success = FALSE;                                
    return;                                                    
  }                                                           
  while (link != lprev) {                                    
  //fprintf(stdout, "DLQ_find(%p). Link = 0x%x\n", (void *)THR_self(), link); fflush(stdout);
    if ((*dlq->cmp) (link, addr, tag, type, mode)) {
      *item = link;                              
      *success = TRUE;                             
    //fprintf(stdout, "DLQ_find(%p): ¡Encontrada!\n", (void *)THR_self()); fflush(stdout); 
      return;                                                
    }                                                        
    link = link->Next;                                      
  }                                                         
//fprintf(stdout, "DLQ_find(%p): No Encontrada :)\n", (void *)THR_self()); fflush(stdout); 
  return;
} 
*/


