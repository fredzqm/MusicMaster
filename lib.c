#include <xc.h>
#include "i2c.h"
#include "lcd4bits.h"
#include "lib.h"

int keyStatus, note;

void playNote() {
    char x = getKeyCharacter();
    note = getNote() / 2;
    if (note == 0){
        CCP1M3 = 1;
    } else {
        CCP1M3 = 0;
    }
    lcd_clear();
    lcd_puts("0x");
    lcd_putch(toHex(keyStatus>>12));
    lcd_putch(toHex(keyStatus>>8));
    lcd_putch(toHex(keyStatus>>12));
    lcd_putch(toHex(keyStatus));
    lcd_goto(0x40); // go the next line
    lcd_putch(x);
    DelayMs(100);
}

int getNote(void)
{
    UpdateKey();
    if ((keyStatus & 0x7330) == 0x7330)
        return 0;
    switch(keyStatus) {
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

unsigned char InChar(void) {
    while (RCIF == 0);
    return RCREG;
}

void OutChar(unsigned char outchr)
{
    while(TXIF == 0); //Wait until Transmit Register is Empty (TXIF = 1).
    TXREG = outchr; //Load character to be sent into Transmit register (TXREG).
}


char getKeyCharacter() {
    UpdateKey();
    switch(keyStatus){
        case 0xefff: return '1';
        case 0xfeff: return '2';
        case 0xffef: return '3';
        case 0xfffe: return 'A';
        case 0xdfff: return '4';
        case 0xfdff: return '5';
        case 0xffdf: return '6';
        case 0xfffd: return 'B';
        case 0xbfff: return '7';
        case 0xfbff: return '8';
        case 0xffbf: return '9';
        case 0xfffb: return 'C';
        case 0x7fff: return '*';
        case 0xf7ff: return '0';
        case 0xff7f: return '#';
        case 0xfff7: return 'D';
        default: return 'X';
    }
    // return ASCII_Table[i]; //Look up ASCII code of key that was pressed. Return 0x00 (ASCII NULL) if no
}


char toHex(char binary){
    binary = binary % 16;
    if (binary < 10) {
        return binary + '0';
    } else {
        return binary - 10 + 'A';
    }
}