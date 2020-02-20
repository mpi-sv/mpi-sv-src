#include <stdio.h>

int main(int argc, char **argv)
{
  // klee_disable_sync_chk(0);
   char temp[10]="hello~";
   printf("%s\n",temp);
   //klee_enable_sync_chk(0);
   return 0;
}
