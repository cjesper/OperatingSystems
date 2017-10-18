#include <stdio.h>

void recursivePrint(char *str);

int main() {
	char str[5] = "Yeah";
	char *s = str;
	recursivePrint(s);
	return 0;	
}

void recursivePrint(char *str) {
	if (*str == '\0')
		return;
	else {
		printf("%c\n", *str++);
		recursivePrint(str);
	}
}
