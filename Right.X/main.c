// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)


#include "../common.h"

#define EEPROMA  0xA0
#define EEPROMB  0xA1
#define GIVE_PROMPT 0xff

void selectGame();
void enterData();

void initialize();
unsigned int ADC_convert(); // Function prototypes
void tokenize(char* command, char** argv);
void handleCommand(char* command);
void handleNote(char* noteSequence);


int width, nextRising;
const int widthMin = 1080;
const int widthRange = 4700;
char inputBuf[50];
char bufCount;
char mode;

void main(void) {
    initialize();
    mode = COMMAND;
    gameMode = DOUBLE;
    if (mode == COMMAND){
        lcd_clear();
        lcd_goto(0);
        lcd_puts("Command Mode");
        lcd_goto(0x40);
        lcd_puts("User Tera Term");
    }
    while (1) {
        switch(mode) {
            case COMMAND:
            case ENTER_DATA:
                enterData();
                break;
            case SELECT_GAME:
                selectGame();
                break;
            case GAME:
                if (gameMode == SINGLE) 
                    singleMode();
                else if (gameMode == DOUBLE)
                    doubleMode();
                break;
            case RESUT_DISPALY:
                break;
        }
    }
}


void selectGame() {
    unsigned int adval, i;
    char tbuff[11];
    char songID;
    for (i = 0 ; i < 50 ; i++){
        DelayMs(5);
        adval = ADC_convert();
        width = widthMin + ( (unsigned long) adval * widthRange )/1023;
    }
    songID = ((long)adval * MAX_NUM_OF_SONG) / 1024;
    lcd_clear();
    lcd_goto(0);
    lcd_puts("Song: ");
    itoa(songID, tbuff); lcd_puts(tbuff);
    switch(gameMode) {
        case SINGLE: lcd_puts(" Single"); break;
        case DOUBLE: lcd_puts(" Double"); break;
    }
    lcd_goto(0x40);
    getSongName(songID, tbuff);
    if (tbuff[0] == '\0')
        lcd_puts("Not exists!");
    else {
        openSong(songID);
        lcd_puts(tbuff);
    }
}

void enterData() {
    if (bufCount == GIVE_PROMPT){
        switch (mode) {
            case COMMAND:
                outString("\n\rMusic Master >:");
                break;
            case ENTER_DATA:
                outString("\n\r>:");
                break;
            default:
                return;
                break;
        }
        bufCount = 0;
    }
    while (!hasInChar()) {
        if (mode != COMMAND || mode!= ENTER_DATA)
            return;
    }
    char input = inChar();
    lcd_clear();
    lcd_putch(input);
    lcd_goto(0x40);
    lcd_putch('0');
    lcd_putch('x');
    lcd_putch(toHex(input/16));
    lcd_putch(toHex(input));
    if (input == 0x0d) {
        inputBuf[bufCount] = '\0';
        switch (mode) {
            case COMMAND:
                handleCommand(inputBuf);
                break;
            case ENTER_DATA:
                handleNote(inputBuf);
                break;
        }
        bufCount = GIVE_PROMPT;
    } else if (input == 0x08) {
        if (bufCount > 0){
            bufCount--;
            outChar(0x08);
            outChar(' ');
            outChar(0x08);
        }
    } else {
        inputBuf[bufCount] = input;
        bufCount++;
        outChar(input);
    }
}

void tokenize(char* command, char** argv) {
    char* itr = command;
    int i = 1;
    argv[0] = itr;
    while(1){
        while (*itr != ' ' && *itr != '\0')
            itr++;
        if (*itr == '\0') {
            break;
        }
        *itr++ = '\0';
        while (*itr == ' ' && *itr != '\0')
            itr++;
        if (*itr == '\0') {
            break;
        }
        argv[i++] = itr;
    }
    argv[i] = '\0';
}

void handleCommand(char* command) {
    char* argv[4];
    char tbuff[11];
    tokenize(command, argv);
    int i = 0;
    if (strcmp(command, "touch")) {
        for (i = 0; i < MAX_NUM_OF_SONG; i++){
            if (!existsSong(i)){
                outString("\n\rCreate song: ");
                outString(argv[1]);
                outString(" Start entering notes:");
                setSongName(i, argv[1]);
                openSong(i);
                mode = ENTER_DATA;
                return;
            }
        }
        outString("\n\rNot enough space for songs. Use rm <Song Name> to free some space.");
    } else if (strcmp(command, "cat")) {
        for (i = 0; i < MAX_NUM_OF_SONG; i++){
            getSongName(i, tbuff);
            if (strcmp(argv[1], tbuff)){
                outString("\n\rDetails of ");
                outString(argv[1]);
                outString(":\n\r");
                openSong(i);
                char read = readSong();
                while(read != END_SONG) {
                    outString(decode(read).name);
                    outChar(' ');
                    read = readSong();
                }
                return;
            }
        }
        outString("\n\rThis song is not found");
    } else if (strcmp(command, "play")) {
        for (i = 0; i < MAX_NUM_OF_SONG; i++){
            getSongName(i, tbuff);
            if (strcmp(argv[1], tbuff)){
                outString("\n\rStart playing ");
                outString(argv[1]);
                openSong(i);
                char read = readSong();
                while(read != END_SONG && mode == COMMAND) {
                    Note n = decode(read);
                    outString("\n\rPlaying  ");
                    outString(n.name);
                    playNote(n.keyEncoding, TIME_FACTOR * n.length);
                    playNote(0, INTERVEL_FACTOR * n.length);
                    read = readSong();
                }
                // playNote(0, 0);
                return;
            }
        }
        outString("\n\rThis song is not found");
    } else if (strcmp(command, "ls")) {
        for (i = 0; i < MAX_NUM_OF_SONG; i++){
            outString("\n\rSong ");
            itoa(i, tbuff); outString(tbuff);
            outString(": ");
            getSongName(i, tbuff); outString(tbuff);
        }
        outString("\n\rThis song is not found");
    } else if (strcmp(command, "rm")) {
        for (i = 0; i < MAX_NUM_OF_SONG; i++){
            getSongName(i, tbuff);
            if (strcmp(argv[1], tbuff)){
                setSongName(i, "");
                outString("\n\rDelete song: ");
                outString(argv[1]);
                return;
            }
        }
        outString("\n\rThis song is not found");
    } else if (strcmp(command, "clear")) {
        for (i = 0; i < MAX_NUM_OF_SONG; i++){
            setSongName(i, "");
        }
    } 
}

void handleNote(char* noteSequence) {
    char* notes[25];
    char tbuff[11];
    tokenize(noteSequence, notes);
    char i = 0, code;
    while(1) {
        outString("\n\rSave this line? (Yes[y], No[n]) :");
        while (!hasInChar()) {
            if (mode != ENTER_DATA)
                return;
        }
        char input = inChar();
        outChar(input);
        if (input == 'y') {
            i = 0;
            while(notes[i] != 0){
                code = encode(notes[i++]);
                writeSong(code);
            }
            break;
        } else if (input == 'n') {
            break;
        }
    }
    while(1) {
        outString("\n\rEnter another line? (Yes[y], No[n]) :");
        while (!hasInChar()) {
            if (mode != ENTER_DATA)
                return;
        }
        char input = inChar();
        outChar(input);
        if (input == 'y') {
            break;
        } else if (input == 'n') {
            endSong();
            mode = COMMAND;
            break;
        }
    }
}

/*
char readEEPROM(unsigned char EEPROMAddr, int address) {
    char data;
    I2C_Start();        // Generate start condition
    while (i2c_WriteTo(EEPROMAddr)); // Send ?Write to Address? byte to all slaves on I2C bus
                        // This routine returns a nonzero value if the addressed
                        // TMP101 fails to acknowledge after the master sends out
                        // the ?Write to address? byte, so the program will hang up in
                        // this loop if there is no response from the TMP101,
                        // repeating this step until an acknowledge is received.

    I2C_SendByte((char)(address >> 8)); // higher byte of address
    I2C_SendByte((char)address); // lower byte of address

    I2C_Restart();      // Generate restart condition between write and read operations
    i2c_ReadFrom(EEPROMAddr);   // Sends ?Read from Address? byte to TMP101
                        // Next two frames from TMP101 contain temperature as
                        // signed 12-bit value
    data = I2C_ReadByte(I2C_MORE);     //get upper 8 bits of temperature (integer portion)
    I2C_Stop();         // Generate stop condition
    return data;
}

char writeEEPROM(unsigned char EEPROMAddr, int address, char data) {
    char ack;
    I2C_Start();        // Generate start condition
    while (i2c_WriteTo(EEPROMAddr)); // Send ?Write to Address? byte to all slaves on I2C bus
                        // This routine returns a nonzero value if the addressed
                        // TMP101 fails to acknowledge after the master sends out
                        // the ?Write to address? byte, so the program will hang up in
                        // this loop if there is no response from the TMP101,
                        // repeating this step until an acknowledge is received.

    I2C_SendByte((char)(address >> 8)); // higher byte of address
    I2C_SendByte((char)address); // lower byte of address
    ack = I2C_SendByte(data);   // set data to EEPROM
    I2C_Stop();         // Generate stop condition
    return data;
}
*/

unsigned int ADC_convert() {
    GO = 1; // Start conversion (“GO” is the GO/DONE* bit)
    while (GO == 1); // Wait here while converting
    return (unsigned int) ADRESH * 256 + ADRESL; // converted 10-bit value (0 -> 1023)
}

void initialize() 
{   
    general_init();

    bufCount = 0;

// -------------------------------------------------------------- comparator module2
    CCP2M3 = 1;CCP2M2 = 0;CCP2M1 = 1;CCP2M0 = 0;
                                //Set CCP2 mode for "Compare with toggle on CCP2 pin" 
                                //See REGISTER 11-1 datasheet
    TRISC1 = 0;                 //Make CCP2 pin an output.
    TRISA5 = 1;
    CCP2IF = 0; 
    CCP2IE = 1;                 //Unmask (enable) Compare Interrupt from CCP2 (Interrupt when CCP2IF flag is set to 1)

// -------------------------------------------------------------- A-D converter
    ADON = 1; // Turn on ADC Module
    ADFM = 1; // Right justify result in ADRES = ADRESH:ADRESL registers
    VCFG1 = 0; VCFG0 = 0; // Use VSS and VDD to set conversion range to 0V-5V
    ADCS1 = 1; ADCS0 = 0; // ADC clock freq set to Fosc/32 = 250kH , T = 4us (assumes Fosc=8MHz)
    
    CHS3 = 0; CHS2 = 1; CHS1 = 0; CHS0 = 0; // set two channel select bits, they won't change during channel switches

// --------------------------------------------------------------- RB interrupt initialization
    IOCB4 = 1;
    IOCB5 = 1;
    RBIF = 0;
    // RBIE = 1;

// ---------------------------------------------------------------
    PEIE = 1;                   //Enable all peripheral interrupts 
    GIE = 1;                    //Globally Enable all interrupts 
}


void interrupt interrupt_handler(void) 
{
    general_interrupt();
    
    if(CCP2IF == 1)
    {
        if (RC1 == 0){
            nextRising = CCPR2 + 40000; // 20ms period
            CCPR2 = width + CCPR2;      // Tick_rate/(2*tone_freq) = 1 MHz/(2*440) = 1136 => generate 440 Hz tone
            RC1 = 1;
          } else {
            CCPR2 = nextRising;
            RC1 = 0;
        }
        CCP2IF = 0;     //Be sure to relax the CCP1 interrupt before returning.
    }
    if (RBIF == 1) {
        if (RB4 == 0) {
            switch(mode) {
                case COMMAND:
                case ENTER_DATA:
                    mode = SELECT_GAME;
                    break;
                case SELECT_GAME:
                    mode = GAME;
                    break;
                case GAME:
                    mode = PAUSE;
                    break;
                case PAUSE:
                    mode = GAME;
                    break;
                case RESUT_DISPALY:
                    mode = SELECT_GAME;
                    break;
                default:
                    mode = SELECT_GAME;
                    break;
            }
        }
        if (RB5 == 0) {
            switch(gameMode) {
                case SINGLE:
                    mode = DOUBLE;
                    break;
                case DOUBLE:
                    mode = SINGLE;
                    break;
                default:
                    mode = SINGLE;
                    break;
            }
        }
        DelayMs(5);
        RBIF = 0;
    }
}
