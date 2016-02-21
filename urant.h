#include <xc.h>

#define BUFF_SIZE 20

char hasInChar();
unsigned char inChar(void);
void outChar(unsigned char);
void outString(char* str);

char bufRead, bufWrite;
char buffer[BUFF_SIZE];
