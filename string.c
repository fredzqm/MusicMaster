#include "string.h"


char toHex(char binary) {
    binary = binary % 16;
    if (binary < 10) {
        return binary + '0';
    } else {
        return binary - 10 + 'A';
    }
}

int strcmp(char* a, char* b) {
    while(*a == *b){
        if (*a == '\0')
            return 1;
        a++;
        b++;
    }
    return 0;
}

void itoa(char n, char* s, int digitNum) {
    s[digitNum] = '\0';
    int i;
    for (i = digitNum - 1; i>=0; i--){
        if (n == 0 && i != digitNum - 1){
            s[i] = ' ';
        } else {
            s[i] = n % 10 + '0';  
            n /= 10;
        }
    }
}