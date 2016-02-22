#include <xc.h>
#include "i2c.h"
#include "lcd4bits.h"
#include "urant.h"
#include "string.h"
#include "song.h"

#define TIME_FACTOR 500000
#define INTERVEL_RATIO 0.1

void pianoMode(); // default
void singleMode();
void doubleMode();
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
