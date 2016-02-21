#include "common.h"

int keyStatus, note;
char mode, bufRead, bufWrite;
char buffer[BUFF_SIZE];

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
    updateKey();
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


char toHex(char binary){
    binary = binary % 16;
    if (binary < 10) {
        return binary + '0';
    } else {
        return binary - 10 + 'A';
    }
}

int strcmp(char* a, char* b) {
    while(*a == *b){
        if (*a == '\0')
            return 1;
        a++;
        b++;
    }
    return 0;
}

void itoa(char n, char* s)
{
    s[3] = '\0';
    int i;
    for (i = 2; i>=0; i--){
        if (n == 0 & i != 2 ){
            s[i] = ' ';
        } else {
            s[i] = n % 10 + '0';  
            n /= 10;
        }
    }
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
    T1CKPS1 = 0; T1CKPS0 = 0;       //Set prescale to divide by 1 yielding a clock tick period of 0.5 microseconds
                                    // 1 sec = 2000000 ticks
                            /*  From Section 6.12 of PIC16F887 Datasheet:
                                    bit 5-4 T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
                                    11 = 1:8 Prescale Value
                                    10 = 1:4 Prescale Value
                                    01 = 1:2 Prescale Value
                                    00 = 1:1 Prescale Value
                            */
// -------------------------------------------------------------- comparator module
    CCP1M3 = 1;CCP1M2 = 0;CCP1M1 = 1;CCP1M0 = 0;
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
        RE0 = 1;
    }
}