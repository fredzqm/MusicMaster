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

void selectGame();
void enterData();


void initialize();
unsigned int ADC_convert(); // Function prototypes
void handleCommand(char* command);

// ---- various of state
// int validChar(char);
int width, nextRising;
const int widthMin = 1080;
const int widthRange = 4700;
char commandBuf[20];
char commandBufIndex;

void main(void) {
    initialize();
    mode = ENTER_DATA;
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
    if (input == 0x0d) {
        outChar('\n');
        outChar('\r');
        commandBuf[commandBufIndex] = '\0';
        outString("Entered Command is: ");
        outString(commandBuf);
        outChar('\n');
        outChar('\r');
        commandBufIndex = 0;
//        handleCommand(commandBuf);
    } else {
        commandBuf[commandBufIndex] = input;
        commandBufIndex++;
        outChar(input);
    }
}

//void handleCommand(char* command) {
//    if (strcmp(command, )) {
//        
//    }
//}


unsigned int ADC_convert() {
    GO = 1; // Start conversion (“GO” is the GO/DONE* bit)
    while (GO == 1); // Wait here while converting
    return (unsigned int) ADRESH * 256 + ADRESL; // converted 10-bit value (0 -> 1023)
}

void initialize() 
{   
    general_init();

    commandBufIndex = 0;

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
