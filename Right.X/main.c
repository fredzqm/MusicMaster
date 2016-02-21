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
#include "song.h"

#define EEPROMA  0xA0
#define EEPROMB  0xA1

void selectGame();
void enterData();

void initialize();
unsigned int ADC_convert(); // Function prototypes
void tokenize(char* command, char** argv);
void handleCommand(char* command);

enum promptMode {
    COMMAND,
    SONG,
    CONFIRM
};
// ---- various of state
// int validChar(char);
int width, nextRising;
const int widthMin = 1080;
const int widthRange = 4700;
char commandBuf[50];
char buffCount;
char promptMode;

void main(void) {
    initialize();
    mode = ENTER_DATA;
    promptMode = COMMAND;

    // mode = 0;
    
    while (1) {
        switch(mode) {
            case SELECT_GAME:
                selectGame();
                break;
            case PLAY_NOTE:
            case TEMP_TEST:
                playNote();
                break;
            case RESUT_DISPALY:
                break;
            case ENTER_DATA:
                enterData();
                break;
        }
    }
}


void selectGame() {
    unsigned int adval, i;
    for (i = 0 ; i < 50 ; i++){
        DelayMs(5);
        adval = ADC_convert();
        width = widthMin + ( (unsigned long) adval * widthRange )/1023;
    }
    unsigned char char0, char1, char2, char3;
    adval = ((unsigned long) adval * 5000 )/1023;
    char0 = adval % 10 ;
    adval = adval / 10;
    char1 = adval % 10 + 0x30; // add 0x30 to convert digit value to ASCII code
    adval = adval / 10;
    char2 = adval % 10 + 0x30;
    adval = adval / 10;
    char3 = adval % 10 + 0x30;
    if (char0 >= 5){
        char1++;
    }
    lcd_clear();
    lcd_goto(0);
    lcd_putch(char3);
    lcd_putch('.');
    lcd_putch(char2);
    lcd_putch(char1);
    lcd_puts("Volts");
}

void enterData() {
    while (!hasInChar());
    char input = inChar();
    lcd_clear();
    lcd_putch(input);
    lcd_goto(0x40);
    lcd_putch('0');
    lcd_putch('x');
    lcd_putch(toHex(input/16));
    lcd_putch(toHex(input));
    switch (promptMode) {
        case COMMAND:
            if (input == 0x0d) {
                commandBuf[buffCount] = '\0';
                outString("\n\rEntered Command is: ");
                outString(commandBuf);
                buffCount = 0;
                handleCommand(commandBuf);
                outString("\n\rMusic Master >:");
            } else if (input == 0x08) {
                if (buffCount > 0){
                    buffCount--;
                    outChar(0x08);
                    outChar(' ');
                    outChar(0x08);
                }
            } else {
                commandBuf[buffCount] = input;
                buffCount++;
                outChar(input);
            }
            break;
        case SONG:

            break;
        case CONFIRM:

            break;
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
    char* argv[20];
    char tbuff[11];
    tokenize(command, argv);
    int i = 0;

    while(argv[i] != 0){
        outString("\n\rArg");
        itoa(i, tbuff);outString(tbuff);
        outString(": ");
        outString(argv[i++]);
    }
    if (strcmp(command, "touch")) {
        for (i = 0; i < MAX_NUM_OF_SONG; i++){
            if (!existsSong(i)){
                setSongName(i, argv[1]);
                openSong(i);
                outString("\n\rCreate song: ");
                outString(argv[1]);
                outString("Start enter notes: ");
                promptMode = SONG;
                return;
            }
        }
        outString("\n\rNot enough space for songs. Use rm <Song Name> to free some space.");
    } else if (strcmp(command, "ls")) {
        for (i = 0; i < MAX_NUM_OF_SONG; i++){
            outString("\n\rSong ");
            itoa(i, tbuff);outString(tbuff);
            outString(": ");
            getSongName(i, tbuff); outString(tbuff);
            return;
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

    buffCount = 0;

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
    // if (RBIF == 1) {
    //     if (RB0 == 0) {
    //             RA5 = 1;
    //         }else
    //     }
    //     DelayMs(5);
    //     RBIF = 0;
    // }
}
