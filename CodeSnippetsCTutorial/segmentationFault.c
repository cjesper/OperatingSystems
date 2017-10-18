#include <stdio.h>

int main(){

	char str[5] = "Nope";
	char *s = str;
	while(*s != '0')
		printf ("%s\n", *s++);

	return 0;

}
