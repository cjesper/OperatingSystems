#include <stdio.h>
#include <stdlib.h>

int main(){

	int n;
	int *ptr = &n;
	n=10;
	int **ptr2ptr = &ptr;
	printf("address of n using &n = %p\n", &n);
	printf ("address of n using ptr = %p\n", ptr);
	printf ("address of n using *ptr2ptr = %p\n", *ptr2ptr);
	printf("value of n = %d\n", n);
	printf("value of n using *ptr = %d\n", *ptr);
	printf ("value of n using **ptr2ptr = %d\n", **ptr2ptr);

	return 0;	
}
