#include <stdio.h>
#include <stdlib.h>

int main(){
    
    char string[5] = "Yeah";
    char *pointerToString = string;
    
    while(*pointerToString != '\0')
    {
        printf("%c\n", *pointerToString++);
    }
    return 0;
}
