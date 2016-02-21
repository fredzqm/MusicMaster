// eeprom_write(address, value);     // Writing value 0x9 to EEPROM address 0xE5        
#include "song.h"


char curSongOffset;
int curSongIndex;

char existsSong(char songID) {
	return (eeprom_read (songID * MAX_SIZE_OF_SONG) == 0);
}

void setSongName(char songID, char* name) {
	int i;
	for (i = 0; i < MAX_LENGTH_OF_SONG_NAME; i++) {
		eeprom_write(songID * MAX_SIZE_OF_SONG + i, name[i]);
		if (*name == '\0')
			return;
	}
}

void getSongName(char songID, char* buf) {
	int i;
	for (i = 0; i < MAX_LENGTH_OF_SONG_NAME; i++) {
		buf[i] = eeprom_read(songID * MAX_SIZE_OF_SONG + i);
		if (buf[i] == '\0')
			return;
	}
	buf[MAX_LENGTH_OF_SONG_NAME] = '\0';
}

void openSong(char songID) {
	curSongOffset = songID * MAX_SIZE_OF_SONG;
	curSongIndex = MAX_LENGTH_OF_SONG_NAME;
}

char writeSong(char note, char length) {
	eeprom_write(curSongOffset + curSongIndex++, note);
}

char readSong() {
	eeprom_read(curSongOffset + curSongIndex++);
}

char endSong() {
	eeprom_write(curSongOffset + curSongIndex, END_SONG);
	curSongIndex = 0;
}

