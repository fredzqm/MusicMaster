#include "i2c.h"
#include "lcd4bits.h"


void playNote(); // default
char toHex(char binary);
int getNote(void);
unsigned char InChar(void);
void OutChar(unsigned char);
char getKeyCharacter();


extern int keyStatus, note;