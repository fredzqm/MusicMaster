#include <xc.h>
#include "i2c.h"
#include "lcd4bits.h"
#include "urant.h"
#include "string.h"

void pianoMode(); // default
int getNote(int keyEncoding);
void playNote(int keyEncoding, long length);
char getKeyCharacter();
void updateKey(void);
void general_init();
void general_interrupt();
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
