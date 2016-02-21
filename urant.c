#include "urant.h"

char hasInChar() {
    return bufRead != bufWrite;
}

unsigned char inChar() {
    char ret = buffer[bufRead];
    bufRead = (bufRead + 1 ) % BUFF_SIZE;
    return ret;
}

void outString(char* str) {
    while(*str != '\0'){
        outChar(*str++);
    }
}


void outChar(unsigned char outchr)
{
    while(TXIF == 0); //Wait until Transmit Register is Empty (TXIF = 1).
    TXREG = outchr; //Load character to be sent into Transmit register (TXREG).
}
