#include <xc.h>

#define MAX_SIZE_OF_SONG 64
#define MAX_LENGTH_OF_SONG_NAME 16
#define MAX_NUM_OF_SONG 4
#define END_SONG 0

typedef struct {
	int keyEncoding;
	char length;
	char* name;
} Note;

char existsSong(char songID);
void setSongName(char songID, char* name);
void getSongName(char songID, char* buf);
void openSong(char songID);
void writeSong(char encoding);
char readSong();
void endSong();

char encode(char* sym);
Note decode(char note);
