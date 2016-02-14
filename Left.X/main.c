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


#include <xc.h>
#include "i2c.h"
#include "lcd4bits.h"

#define SENSEOR_A  0x90

void initialize();
void testTemperature(unsigned char TMP101_address);
void configTemSensor(char TMP101_address);
void UpdateKey(void); //Function prototypes for the three functions.
char getKeyCharacter();
int getNote(void);
unsigned char InChar(void);
void OutChar(unsigned char);
int validChar(char);
char toHex(char binary);

int note;
int keyStatus;

void main(void) {
    initialize();
    // Sensor = SENSEOR_A;
    while (1) {
        char temp_rem, temp;
        testTemperature(SENSEOR_A); // TMP101 with address 1001000
        DelayMs(100);
        // int i;
        // for (i = 0; i < 0; i++){
        //     char x = getKeyCharacter();
        //     note = getNote() / 2;
        //     if (note == 0){
        //         CCP1M3 = 1;
        //     } else {
        //         CCP1M3 = 0;
        //     }
        //     lcd_clear();
        //     lcd_goto(0);
        //     lcd_putch('0');
        //     lcd_putch('x');
        //     lcd_putch((keyStatus>>12)% 16 - 10 + 'A');
        //     lcd_putch((keyStatus>>8) % 16 - 10 + 'A');
        //     lcd_putch((keyStatus>>4) % 16 - 10 + 'A');
        //     lcd_putch(keyStatus % 16 - 10 + 'A');
        //     lcd_goto(0x40); // go the next line
        //     lcd_putch(x);
        //     lcd_puts("Sensor: ");
        //     DelayMs(100);
        // }
    }
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

void UpdateKey() {
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

unsigned char InChar(void) {
    while (RCIF == 0);
    char read = RCREG;
    char check = read;
    char count = 0;
    for (char i = 0 ; i < 8; i++) {
        count += check % 2;
        check /= 2;
    }
    if (count % 2) {
        return read & 0x7f; // the parity bit check success
    }
    return 0; // malfunct data, parity check fail
}

void OutChar(unsigned char outchr)
{
    char check = outchr;
    char count = 0;
    for (char i = 0 ; i < 7; i++) {
        count += check % 2;
        check /= 2;
    }
    if (count % 2) {
      outchr = outchr & 0x7f; // Set the parity bit low
    } else {
      outchr = outchr | 0x80; // Set the parity bit high
    }
    while(TXIF == 0); //Wait until Transmit Register is Empty (TXIF = 1).
    TXREG = outchr; //Load character to be sent into Transmit register (TXREG).
}

//int validChar(char c) {
//    for (int i = 0; i < 16 ; i++){
//        if(c == ASCII_Table[i]){
//            return 1;
//        }
//    }
//    return 0;
//}

void initialize() 
{
    
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
    BRG16 = 1; BRGH=1; SPBRGH = 0; SPBRG = 25; //Config UART for 9600 Baud

// -------------------------------------------------------------- timer1
    TMR1GE=0;   TMR1ON = 1;         //Enable TIMER1 (See Fig. 6-1 TIMER1 Block Diagram in PIC16F887 Data Sheet)
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
    CCP1M3 = 0;CCP1M2 = 0;CCP1M1 = 1;CCP1M0 = 0;
                                //Set CCP1 mode for "Compare with toggle on CCP1 pin" 
                                //See REGISTER 11-1 datasheet
    TRISC2 = 0;                 //Make CCP1 pin an output.
    CCP1IF = 0; 
    CCP1IE = 1;                 //Unmask (enable) Compare Interrupt from CCP1 (Interrupt when CCP1IF flag is set to 1)

 // --------------------------------------------------------------- set this to 12-bit mode
    lcd_init();
    I2C_Initialize();
    configTemSensor(SENSEOR_A);
 // --------------------------------------------------------------- RB interrupt initialization
//    IOCB0 = 1;
//    RBIF = 0;
    // RBIE = 1;
 // ---------------------------------------------------------------
    PEIE = 1;                   //Enable all peripheral interrupts 
    GIE = 1;                    //Globally Enable all interrupts 
}


void interrupt interrupt_handler(void) 
{
    if(CCP1IF == 1)
    {
        CCPR1 = note + CCPR1; 
        CCP1IF = 0;     //Be sure to relax the CCP1 interrupt before returning.
    }
    // if (RBIF == 1) {
    //     if (RB0 == 0) {
    //         if (Sensor == SENSEOR_A){
    //             RA5 = 1;
    //         }else
    //             Sensor = SENSEOR_A;
    //     }
    //     DelayMs(5);
    //     RBIF = 0;
    // }
}




void testTemperature(unsigned char TMP101_address) {
    char temp, temp_rem;
    I2C_Start();        // Generate start condition
    while (i2c_WriteTo(TMP101_address)); // Send ?Write to Address? byte to all slaves on I2C bus
                        // This routine returns a nonzero value if the addressed
                        // TMP101 fails to acknowledge after the master sends out
                        // the ?Write to address? byte, so the program will hang up in
                        // this loop if there is no response from the TMP101,
                        // repeating this step until an acknowledge is received.

    I2C_SendByte(0x00); // Sets pointer register to 00 (Temperature register) for
                        //the addressed TMP101.  (See Table 2 TMP101 Datasheet)
    I2C_Restart();      // Generate restart condition between write and read operations
    i2c_ReadFrom(TMP101_address);   // Sends ?Read from Address? byte to TMP101
                        // Next two frames from TMP101 contain temperature as
                        // signed 12-bit value
    temp = I2C_ReadByte(I2C_MORE);     //get upper 8 bits of temperature (integer portion)
    temp_rem = I2C_ReadByte(I2C_LAST); //get lower 4 bits temperature (fractional portion)
    I2C_Stop();         // Generate stop condition

// display result
    char sign_char, dig0_char, dig1_char, dig2_char, tenth_char, hundredth_char;
    
    if (temp & 0x80)            // If temp is negative make sign character ?-?
    {                           // and negate temp to make it positive.
        temp = -temp;
        sign_char = '-';
    } else {
        sign_char = '+';
    }    // Otherwise temp is positive and make sign character ?+?
    dig0_char = temp % 10;      // Extract temperature as 3 decimal digits
    temp = temp / 10;
    dig1_char = temp % 10;
    temp = temp / 10;
    dig2_char = temp % 10;
    temp_rem = 0x0f & (temp_rem >> 4); // put those four bit at lower nibble
    temp_rem = temp_rem * 10;
    tenth_char = (temp_rem & 0xf0) / 16;
    temp_rem = (temp_rem & 0x0f) * 10;
    hundredth_char = (temp_rem & 0xf0) / 16;
    lcd_clear();
    lcd_goto(0);
    lcd_puts("Temp: ");
    lcd_putch(sign_char);
    lcd_putch(dig2_char + 0x30); // Turn decimal digits into ASCII characters before
    lcd_putch(dig1_char + 0x30); // sending out to the LCD panel.
    lcd_putch(dig0_char + 0x30);
    lcd_putch('.');
    lcd_putch(tenth_char + 0x30);
    lcd_putch(hundredth_char + 0x30);
    lcd_putch(0xdf);
    lcd_putch('C');
    
}

void configTemSensor(char TMP101_address) {
    // char ack = 1;
    // while (ack){ // if ack is high, try this again.
        I2C_Start();        // Generate start condition
         while (i2c_WriteTo(TMP101_address)); // Send ?Write to Address? byte to all slaves on I2C bus
                            // This routine returns a nonzero value if the addressed
                            // TMP101 fails to acknowledge after the master sends out
                            // the ?Write to address? byte, so the program will hang up in
                            // this loop if there is no response from the TMP101,
                            // repeating this step until an acknowledge is received.
        RA5 = I2C_SendByte(0x01); // Sets pointer register to 01 (configure register) for
    // }
                        //the addressed TMP101.  (See Table 2 TMP101 Datasheet)
    I2C_SendByte(0x60); // Sets pointer register to 00 (configure register) for
    I2C_Stop();         // Generate stop condition

}


char toHex(char binary){
    binary = binary % 16;
    if (binary < 10) {
        return binary + '0';
    } else {
        return binary - 10 + 'A';
    }
}