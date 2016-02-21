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

void writeSong(char encoding) {
	eeprom_write(curSongOffset + curSongIndex++, encoding);
}

char readSong() {
	eeprom_read(curSongOffset + curSongIndex++);
}

void endSong() {
	eeprom_write(curSongOffset + curSongIndex, END_SONG);
	curSongIndex = 0;
}



char encode(char* sym) {
	char tone;
	if (*sym == '#'){
		tone = 0x00;
		sym++;
	} else if (*sym == '*'){
		tone = 0x10;
		sym++;
	} else {
		tone = 0x08;
	}
	char length = *sym++ - '0';
	if (length < 1 || length > 7)
		return END_SONG; // error
	tone += length;
	if (*sym == '\\' && *(sym+1) == '\0'){
		tone |= 0x00; // 32th note
	} else if (*sym == '\0'){
		tone |= 0x40; // 16th note
	} else if (*sym == '-' && *(sym+1) == '\0'){
		tone |= 0x80; // 8th note
	} else if (*sym == '-' && *(sym+1) == '-' && *(sym+2) == '\0'){
		tone |= 0xc0; // 4th note
	} else {
		return END_SONG;
	}
	return tone;
}

//Note decode(char note) {
//	Note n;
//
//	switch(keyStatus) {
//        case 0x6fff: return 3058; // *1 (1A)
//        case 0x7eff: return 2724; // *2 (2A)
//        case 0x7fef: return 2427; // *3 (3A)
//        case 0x5fff: return 2291; // *4 (4A)
//        case 0x7dff: return 2041; // *5 (5A)
//        case 0x7fdf: return 1818; // *6 (6A)
//        case 0x3fff: return 1620; // *7 (7A)
//
//        case 0xefff: return 1529; // 1
//        case 0xfeff: return 1362; // 2
//        case 0xffef: return 1213; // 3
//        case 0xdfff: return 1145; // 4
//        case 0xfdff: return 1020; // 5
//        case 0xffdf: return 909; // 6
//        case 0xbfff: return 810; // 7
//
//        case 0xef7f: return 764; // #1 (1B)
//        case 0xfe7f: return 681; // #2 (2B)
//        case 0xff6f: return 607; // #3 (3B)
//        case 0xdf7f: return 573; // #4 (4B)
//        case 0xfd7f: return 510; // #5 (5B)
//        case 0xff5f: return 454; // #6 (6B)
//        case 0xbf7f: return 405; // #7 (7B)
//
//        default: return 0; // not found
//    }
//}