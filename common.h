#include <xc.h>
#include "i2c.h"
#include "lcd4bits.h"
#include "urant.h"
#include "string.h"
#include "song.h"

#define INTERVEL_RATIO 0.1
#define LINE_WIDTH 16

void pianoMode(); // default
void game();
void result();
int getNote(int keyEncoding);
void updateNote(int keyEncoding);
char playNote(int keyEncoding, long length);
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
