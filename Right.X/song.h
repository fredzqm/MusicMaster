#include <xc.h>

#define MAX_SIZE_OF_SONG 64
#define MAX_LENGTH_OF_SONG_NAME 16
#define MAX_NUM_OF_SONG 4
#define END_SONG 0

char existsSong(char songID);
void setSongName(char songID, char* name);
void getSongName(char songID, char* buf);
void openSong(char songID);
char writeSong(char note, char length);
char readSong();
char endSong();
