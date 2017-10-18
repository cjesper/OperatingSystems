#include <stdio.h>
#include <stdlib.h>

int main(){
	
	int n;
	int *ptr = &n;
	n=10;
	printf("address of n using &n = %x\n", &n);
        printf ("address of n using ptr = %p\n", ptr);
       	printf ("value of n = %d\n", n);
	printf ("value of n using *ptr (dereference) = %d\n", *ptr);
	return 0;	
}
