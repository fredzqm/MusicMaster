#include "i2c.h"
#include "lcd4bits.h"



void playNote(); // default
char toHex(char binary);
int getNote(void);
unsigned char InChar(void);
void OutChar(unsigned char);
char getKeyCharacter();
void UpdateKey(void);
void general_init();

enum Mode 
{ 
  SELECT_GAME, 
  PLAY_NOTE,
  TEMP_TEST,
  RESUT_DISPALY,
  ENTER_DATA
};
 
extern int keyStatus, note;
extern char mode, flag, buffStart, buffEnd;
extren char buffer[20];
