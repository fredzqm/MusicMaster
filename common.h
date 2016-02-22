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
  COMMAND,
  ENTER_DATA,
  SELECT_GAME, 
  GAME,
  PAUSE,
  RESUT_DISPALY
};

enum GameMode 
{ 
  SINGLE,
  DOUBLE
};
 
extern int keyStatus, note;
extern char mode, gameMode;
