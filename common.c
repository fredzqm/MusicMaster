#include "common.h"
// Red BLue Green
#define RED RE0 = 1; RE1 = 0; RE2 = 0; 
#define GREEN RE0 = 0; RE1 = 0; RE2 = 1; 
#define YELLOW RE0 = 1; RE1 = 1; RE2 = 0; 
#define BLUE RE0 = 0; RE1 = 1; RE2 = 0; 
#define DARK RE0 = 0; RE1 = 0; RE2 = 0; 


int keyStatus, note, totalScore, nCount;
int timerCounter;
char mode, gameMode;


char playNote(int keyEncoding, long length) {
    signed char scor = 0;
    long clock = getTime();
    updateKey();
    if (keyEncoding == keyStatus){
        scor = -1;
        RED
    }
    if (gameMode == SINGLE) {
        updateNote(keyStatus);
    } else {
        updateNote(keyEncoding);
    }
    DelayMs(50);
    updateKey();
    if (scor == 0 && keyEncoding == keyStatus){
        scor = 4;
        GREEN
    }
    if (gameMode == SINGLE) {
        updateNote(keyStatus);
    }

    DelayMs(100);
    updateKey();
    if (scor == 0 && keyEncoding == keyStatus){
        scor = 3;
        YELLOW
    }
    if (gameMode == SINGLE) {
        updateNote(keyStatus);
    }

    clock += length;
    char x = 0;
    while(getTime() - clock < 0){
        DelayMs(10);
        updateKey();
        if (scor == 0 && keyEncoding == keyStatus){
            scor = 1;
            BLUE
        }
        if (gameMode == SINGLE) {
            updateNote(keyStatus);
        }
    }
    if (scor == -1)
        DARK
    return scor;
}

void result() {
    char tbuff[5];
    lcd_clear();
    lcd_goto(0);
    lcd_puts("Notes: ");
    itoa(nCount, tbuff, 5); lcd_puts(tbuff);
    lcd_goto(0x40);
    lcd_puts("Score: ");
    itoa(totalScore, tbuff, 5); lcd_puts(tbuff);
    DelayMs(100);
}


void updateNote(int keyEncoding){
    note = getNote(keyEncoding);
    if (note == 0){
        CCP1M3 = 1;
    } else {
        CCP1M3 = 0;
    }
}

int getNote(int keyEncoding)
{
    updateKey();
    if ((keyEncoding & 0x7330) == 0x7330)
        return 0;
    switch(keyEncoding) {
        case 0x6fff: return 1529; // 1A
        case 0x7eff: return 1362; // 2A
        case 0x7fef: return 1214; // 3A
        case 0x5fff: return 1145; // 4A
        case 0x7dff: return 1021; // 5A
        case 0x7fdf: return 909; // 6A
        case 0x3fff: return 810; // 7A

        case 0xefff: return 765; // 1
        case 0xfeff: return 681; // 2
        case 0xffef: return 607; // 3
        case 0xdfff: return 573; // 4
        case 0xfdff: return 510; // 5
        case 0xffdf: return 455; // 6
        case 0xbfff: return 405; // 7

        case 0xef7f: return 382; // 1B
        case 0xfe7f: return 341; // 2B
        case 0xff6f: return 303; // 3B
        case 0xdf7f: return 286; // 4B
        case 0xfd7f: return 255; // 5B
        case 0xff5f: return 227; // 6B
        case 0xbf7f: return 202; // 7B

        default: return 0; // not found
    }
}

char getKeyCharacter() {
    updateKey();
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

void updateKey() {
    // keyStatus 16bit represent *741 0852 #693 DCBA
    PORTA = 0b1110; //Check first COL for key depression
    keyStatus = (((int)PORTB << 12) & 0xf000 ) ;
    PORTA = 0b1101; //Check second COL for key depression
    keyStatus |= ((int)PORTB << 8) & 0x0f00;
    PORTA = 0b1011; //Check third COL for key depression
    keyStatus |= (PORTB << 4) & 0x00f0;
    PORTA = 0b0111; //Check fourth COL for key depression
    keyStatus |= PORTB & 0x000f;
}

long getTime(){
    return ( (long)timerCounter <<16) + TMR1;
}


void general_init() {
    ANSEL = 0; // disable analog input
    ANSELH = 0;
    nRBPU = 0; // enable inherent pull up resistor for PORTB
    TRISB = 0xff; // set PORTB as input
    TRISA = 0; // set PORTA as output, for debugging purpose
    TRISD = 0; //Make RD3:0 outputs (LEDs connected to RD3:0)
    TRISE = 0;

// -------------------------------------------------------------- UART

    CREN = 1; // Enable receive side of UART
    SPEN = 1; SYNC = 0; TXEN = 1; //Enable transmit side of UART for asynchronous operation
    BRG16 = 1; BRGH= 0; SPBRGH = 0; SPBRG = 51; //Config UART for 9600 Baud
    // system clock is 8MHz
    RCIF = 0;
    RCIE = 1;
// -------------------------------------------------------------- timer1
    TMR1GE = 0;   TMR1ON = 1;         //Enable TIMER1 (See Fig. 6-1 TIMER1 Block Diagram in PIC16F887 Data Sheet)
    TMR1CS = 0;                     //Select internal clock whose frequency is Fosc/4 = 2 MHz, where Fosc = 8 MHz
    T1CKPS1 = 0; T1CKPS0 = 0;       //Set prescale to divide by 1 yielding a clock tick period of 0.5 microsecond, scores
    timerCounter = 0;
    TMR1IF = 0;
    TMR1IE = 1;
// -------------------------------------------------------------- comparator module
    CCP1M3 = 1; CCP1M2 = 0;CCP1M1 = 1;CCP1M0 = 0;
                                //Set CCP1 mode for "Compare with toggle on CCP1 pin" 
                                //See REGISTER 11-1 datasheet
    TRISC2 = 0;                 //Make CCP1 pin an output.
    CCP1IF = 0; 
    CCP1IE = 1;                 //Unmask (enable) Compare Interrupt from CCP1 (Interrupt when CCP1IF flag is set to 1)

    lcd_init();

    bufRead = 0;
    bufWrite = 0;

}

void general_interrupt() {
    if(CCP1IF == 1)
    {
        CCPR1 = note + CCPR1; 
        CCP1IF = 0;     //Be sure to relax the CCP1 interrupt before returning.
    }
    if(RCIF == 1)
    {
        buffer[bufWrite] = RCREG;
        bufWrite = (bufWrite + 1 ) % BUFF_SIZE;
        RCIF = 0;
    }
    if(TMR1IF == 1) {
        timerCounter++;
        TMR1IF = 0;
        RE1 =1;
    }
}