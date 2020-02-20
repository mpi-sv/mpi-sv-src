#include <stdio.h>

int main(){
int i=0;
int *ptr=&i;

free(ptr);
printf("free finished");
return 0;
}
