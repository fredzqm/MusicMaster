// eeprom_write(address, value);     // Writing value 0x9 to EEPROM address 0xE5        
#include "song.h"


char curSongOffset;
int curSongIndex;

char existsSong(char songID) {
	return (eeprom_read (songID * MAX_SIZE_OF_SONG) != '\0');
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
	if (*sym == '*'){
		tone = 0x00; // low octate, 0b--00 ----
		sym++;
	} else if (*sym == '#'){
		tone = 0x20; // high octate, 0b--10 ----
		sym++;
	} else {
		tone = 0x10; // middle octate, 0b--01 ----
	}
	char length = *sym++ - '0';
	if (length < 1 || length > 7)
		return END_SONG; // error
	tone += length;
	if (*sym == '\\' && *(sym+1) == '\0'){
		tone |= 0x00; // 32th note, 1/2 sec, 0b00-- ----
	} else if (*sym == '\0'){
		tone |= 0x40; // 16th note, 1 sec, 0b01-- ----
	} else if (*sym == '-' && *(sym+1) == '\0'){
		tone |= 0x80; // 8th note, 2 sec, 0b10-- ----
	} else if (*sym == '-' && *(sym+1) == '-' && *(sym+2) == '\0'){
		tone |= 0xc0; // 4th note, 4 sec, 0b11-- ----
	} else {
		return END_SONG;
	}
	return tone;
}


Note decode(char note) {
	Note n;
	switch(note & 0xc0){
		case 0x00:
			n.length = 1;
			break;
		case 0x40:
			n.length = 2; // this should be around 1 sec
			break;
		case 0x80:
			n.length = 4;
			break;
		case 0xc0:
			n.length = 8;
			break;
	}
	switch(note & 0x37) {
       case 0x01: 
       		n.period = 3058; 
       		n.name[1] = '*';
       		n.name[2] = '1';
       		n.name[3] = '\0';
       		break; // *1 (1A)
       case 0x02: 
       		n.period = 2724; 
       		n.name[1] = '*';
       		n.name[2] = '2';
       		n.name[3] = '\0';
       		break; // *2 (2A)
       case 0x03: 
       		n.period = 2427; 
       		n.name[1] = '*';
       		n.name[2] = '3';
       		n.name[3] = '\0';
       		break; // *3 (3A)
       case 0x04: 
       		n.period = 2291; 
       		n.name[1] = '*';
       		n.name[2] = '4';
       		n.name[3] = '\0';
       		break; // *4 (4A)
       case 0x05: 
       		n.period = 2041; 
       		n.name[1] = '*';
       		n.name[2] = '5';
       		n.name[3] = '\0';
       		break; // *5 (5A)
       case 0x06: 
       		n.period = 1818; 
       		n.name[1] = '*';
       		n.name[2] = '6';
       		n.name[3] = '\0';
       		break; // *6 (6A)
       case 0x07: 
       		n.period = 1620; 
       		n.name[1] = '*';
       		n.name[2] = '7';
       		n.name[3] = '\0';
       		break; // *7 (7A)

       case 0x11: 
       		n.period = 1529; 
       		n.name[1] = '1';
       		n.name[2] = '\0';
       		break; // 1
       case 0x12: 
       		n.period = 1362; 
       		n.name[1] = '2';
       		n.name[2] = '\0';
       		break; // 2
       case 0x13: 
       		n.period = 1213; 
       		n.name[1] = '3';
       		n.name[2] = '\0';
       		break; // 3
       case 0x14: 
       		n.period = 1145; 
       		n.name[1] = '4';
       		n.name[2] = '\0';
       		break; // 4
       case 0x15: 
       		n.period = 1020; 
       		n.name[1] = '5';
       		n.name[2] = '\0';
       		break; // 5
       case 0x16: 
       		n.period = 909;  
       		n.name[1] = '6';
       		n.name[2] = '\0';
       		break; // 6
       case 0x17: 
       		n.period = 810;  
       		n.name[1] = '7';
       		n.name[2] = '\0';
       		break; // 7

       case 0x21: 
       		n.period = 764;  
       		n.name[1] = '#';
       		n.name[2] = '1';
       		n.name[3] = '\0';
       		break; // #1 (1B)
       case 0x22: 
       		n.period = 681;  
       		n.name[1] = '#';
       		n.name[2] = '2';
       		n.name[3] = '\0';
       		break; // #2 (2B)
       case 0x23: 
       		n.period = 607;  
       		n.name[1] = '#';
       		n.name[2] = '3';
       		n.name[3] = '\0';
       		break; // #3 (3B)
       case 0x24: 
       		n.period = 573;  
       		n.name[1] = '#';
       		n.name[2] = '4';
       		n.name[3] = '\0';
       		break; // #4 (4B)
       case 0x25: 
       		n.period = 510;  
       		n.name[1] = '#';
       		n.name[2] = '5';
       		n.name[3] = '\0';
       		break; // #5 (5B)
       case 0x26: 
       		n.period = 454;  
       		n.name[1] = '#';
       		n.name[2] = '6';
       		n.name[3] = '\0';
       		break; // #6 (6B)
       case 0x27: 
       		n.period = 405;  
       		n.name[1] = '#';
       		n.name[2] = '7';
       		n.name[3] = '\0';
       		break; // #7 (7B)

       default: n.period = END_SONG; // not found
   }
   return n;
}