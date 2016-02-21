// eeprom_write(address, value);     // Writing value 0x9 to EEPROM address 0xE5	
#include "song.h"
// #include "../common.h"

char curSongOffset;

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
	curSongOffset = songID * MAX_SIZE_OF_SONG + MAX_LENGTH_OF_SONG_NAME;
}

void writeSong(char encoding) {
	// outString("\n\rWrite at: 0x");
	// outChar(toHex(curSongOffset>>4));
	// outChar(toHex(curSongOffset));
	eeprom_write(curSongOffset++, encoding);
}

char readSong() {
	// outString("\n\rRead at: 0x");
	// outChar(toHex(curSongOffset>>4));
	// outChar(toHex(curSongOffset));
	return eeprom_read(curSongOffset++);
}

void endSong() {
	eeprom_write(curSongOffset, END_SONG);
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

/*
	switch(keyEncoding) {
        case 0x6fff: return 3058; // 1A
        case 0x7eff: return 2724; // 2A
        case 0x7fef: return 2427; // 3A
        case 0x5fff: return 2291; // 4A
        case 0x7dff: return 2041; // 5A
        case 0x7fdf: return 1818; // 6A
        case 0x3fff: return 1620; // 7A

        case 0xefff: return 1529; // 1
        case 0xfeff: return 1362; // 2
        case 0xffef: return 1213; // 3
        case 0xdfff: return 1145; // 4
        case 0xfdff: return 1020; // 5
        case 0xffdf: return 909; // 6
        case 0xbfff: return 810; // 7

        case 0xef7f: return 764; // 1B
        case 0xfe7f: return 681; // 2B
        case 0xff6f: return 607; // 3B
        case 0xdf7f: return 573; // 4B
        case 0xfd7f: return 510; // 5B
        case 0xff5f: return 454; // 6B
        case 0xbf7f: return 405; // 7B

        default: return 0; // not found
    }
}

*/

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
		default:
			return;
	}
	switch(note & 0x37) {
		case 0x01: 
			n.name[1] = '*';
			n.name[2] = '1';
			n.name[3] = '\0';
			break; // *1 (1A)
		case 0x02: 
			n.name[1] = '*';
			n.name[2] = '2';
			n.name[3] = '\0';
			break; // *2 (2A)
		case 0x03: 
			n.name[1] = '*';
			n.name[2] = '3';
			n.name[3] = '\0';
			break; // *3 (3A)
		case 0x04: 
			n.name[1] = '*';
			n.name[2] = '4';
			n.name[3] = '\0';
			break; // *4 (4A)
		case 0x05: 
			n.name[1] = '*';
			n.name[2] = '5';
			n.name[3] = '\0';
			break; // *5 (5A)
		case 0x06: 
			n.name[1] = '*';
			n.name[2] = '6';
			n.name[3] = '\0';
			break; // *6 (6A)
		case 0x07: 
			n.name[1] = '*';
			n.name[2] = '7';
			n.name[3] = '\0';
			break; // *7 (7A)

		case 0x11: 
			n.name[1] = '1';
			n.name[2] = '\0';
			break; // 1
		case 0x12: 
			n.name[1] = '2';
			n.name[2] = '\0';
			break; // 2
		case 0x13: 
			n.name[1] = '3';
			n.name[2] = '\0';
			break; // 3
		case 0x14: 
			n.name[1] = '4';
			n.name[2] = '\0';
			break; // 4
		case 0x15: 
			n.name[1] = '5';
			n.name[2] = '\0';
			break; // 5
		case 0x16: 
			n.name[1] = '6';
			n.name[2] = '\0';
			break; // 6
		case 0x17: 
			n.name[1] = '7';
			n.name[2] = '\0';
			break; // 7

		case 0x21: 
			n.name[1] = '#';
			n.name[2] = '1';
			n.name[3] = '\0';
			break; // #1 (1B)
		case 0x22: 
			n.name[1] = '#';
			n.name[2] = '2';
			n.name[3] = '\0';
			break; // #2 (2B)
		case 0x23: 
			n.name[1] = '#';
			n.name[2] = '3';
			n.name[3] = '\0';
			break; // #3 (3B)
		case 0x24: 
			n.name[1] = '#';
			n.name[2] = '4';
			n.name[3] = '\0';
			break; // #4 (4B)
		case 0x25: 
			n.name[1] = '#';
			n.name[2] = '5';
			n.name[3] = '\0';
			break; // #5 (5B)
		case 0x26: 
			n.name[1] = '#';
			n.name[2] = '6';
			n.name[3] = '\0';
			break; // #6 (6B)
		case 0x27: 
			n.name[1] = '#';
			n.name[2] = '7';
			n.name[3] = '\0';
			break; // #7 (7B)

		default:
			return; // not found
   }
   return n;
}