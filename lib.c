#include <xc.h>
#include "Left.X/i2c.h"
#include "Left.X/lcd4bits.h"
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