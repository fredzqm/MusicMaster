#include <xc.h>
#include "i2c.h"
#include "lcd4bits.h"

#define BUFF_SIZE 20

void pianoMode(); // default
char toHex(char binary);
int getNote(int keyEncoding);
void playNote(int keyEncoding, long length);
char hasInChar();
unsigned char inChar(void);
void outChar(unsigned char);
void outString(char* str);
char getKeyCharacter();
void updateKey(void);
void general_init();
void general_interrupt();
int strcmp(char* a, char* b);
void itoa(char n, char* s);
long getTime();

enum Mode 
{ 
  SELECT_GAME, 
  GAME,
  TEMP_TEST,
  RESUT_DISPALY,
  ENTER_DATA
};
 
extern int keyStatus, note;
extern char mode, flag, buffStart, buffEnd;
extern char buffer[BUFF_SIZE];
