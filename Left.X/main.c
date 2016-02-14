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
void get_temperature(unsigned char TMP101_address, char *temp, char *temp_rem);
void configTemSensor(char TMP101_address);
unsigned void UpdateKey(void); //Function prototypes for the three functions.
char getKeyCharacter();
void updateNote(void);
unsigned char InChar(void);
void OutChar(unsigned char);
int validChar(char);

// const unsigned char step_tbl[8] = {0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001, 0b0001}; // High-Torque Stepping Sequence (See Table)
const unsigned char Keycode_Table[17] = {0xEE, 0xDE, 0xBE, 0x7E, 0xED, 0xDD, 0xBD, 0x7D, 0xEB, 0xDB, 0xBB, 0x7B, 0xE7, 0xD7, 0xB7, 0x77, 0x00};
const unsigned char ASCII_Table[17] = {'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D', -1};
int period;
int keyStatus;

void main(void) {
    char temp, temp_rem;
    char sign_char, dig0_char, dig1_char, dig2_char, tenth_char, hundredth_char;
    initialize();
    // Sensor = SENSEOR_A;
    period = 0;
    while (1) {
//        // get_temperature(SENSEOR_A, &temp, &temp_rem); // TMP101 with address 1001000
        char x = getKeyCharacter();
        lcd_clear();
        lcd_goto(0);
        lcd_putch('C');
        lcd_putch(x);
        
        lcd_goto(0x40); // go the next line
        lcd_putch(x);
        lcd_puts("Sensor: ");
        DelayMs(100);
    }
}

const unsigned char Keycode_Table[17] = 
{0xEE, 0xDE, 0xBE, 0x7E, 0xED, 0xDD, 0xBD, 0x7D, 0xEB, 0xDB, 0xBB, 0x7B, 0xE7, 0xD7, 0xB7, 0x77, 0x00};
const unsigned char ASCII_Table[17] = 
{'1' , '2' , '3' , 'A' , '4' , '5' , '6' , 'B' , '7' , '8' , '9' , 'C' , '*' , '0' , '#' , 'D' , -1};
void updateNote(void)
{
    if (RB1 == 0) {
        if (RB0 == 0) {
            note = 2273; //440Hz, A4
        } else {
            note = 4545; //220Hz, A3
        }
        CCP1M3 = 0;
    } else if (RB2 == 0) {
        if (RB0 == 0) {
            note = 2025; //493.88Hz, B4
        } else {
            note = 4050; //246.94Hz, B3
        }
        CCP1M3 = 0;
    } else if (RB3 == 0) {
        if (RB0 == 0) {
            note = 1911; //523.26Hz, C4
        } else {
            note = 3822; //261.63Hz, C3
        }
        CCP1M3 = 0;
    } else {
        note = 0;
        CCP1M3 = 1;
    }
}

unsigned char UpdateKey()
{
    // keyStatus 16bit represent *741 0852 #693 DCBA
    PORTA = 0b1110; //Check first COL for key depression
    keyStatus = (((int)PORTB << 12) & 0xf000 ) ;
    PORTA = 0b1101; //Check second COL for key depression
    keyStatus |= ((int)PORTB << 8) & 0x0f00;
    PORTA = 0b1011; //Check third COL for key depression
    keyStatus = (PORTB << 4) & 0x00f0 ) ;
    PORTA = 0b0111; //Check fourth COL for key depression
    keyStatus |= PORTB & 0x000f;
}

char getKeyCharacter() {
    UpdateKey();
    switch(keyStatus){
        case 0x1000: return '1';
        case 0x0100: return '2';
        case 0x0010: return '3';
        case 0x0001: return 'A';
        case 0x2000: return '4';
        case 0x0200: return '5';
        case 0x0020: return '6';
        case 0x0002: return 'B';
        case 0x4000: return '7';
        case 0x0400: return '8';
        case 0x0040: return '9';
        case 0x0004: return 'C';
        case 0x8000: return '*';
        case 0x0800: return '0';
        case 0x0080: return '#';
        case 0x0008: return 'D';
    }
    return ASCII_Table[i]; //Look up ASCII code of key that was pressed. Return 0x00 (ASCII NULL) if no
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

int validChar(char c) {
    for (int i = 0; i < 16 ; i++){
        if(c == ASCII_Table[i]){
            return 1;
        }
    }
    return 0;
}

void initialize() 
{
    
    ANSEL = 0; // disable analog input
    ANSELH = 0;
    nRBPU = 0; // enable inherent pull up resistor for PORTB
    TRISB = 0xff; // set PORTB as input
    TRISA = 0; // set PORTA as output, for debugging purpose
    TRISD = 0b11110000; //Make RD3:0 outputs (LEDs connected to RD3:0)
    // RD3 = 0;
    TRISA = 0; //Make RA3:0 outputs (to drive the columns of the 4x4 keypad)
    RA5 = 1;

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
    // TRISC2 = 0;                 //Make CCP1 pin an output.
    CCP1IF = 0; 
    CCP1IE = 1;                 //Unmask (enable) Compare Interrupt from CCP1 (Interrupt when CCP1IF flag is set to 1)

 // --------------------------------------------------------------- set this to 12-bit mode
    I2C_Initialize();
    // configTemSensor(SENSEOR_A);
    lcd_init();
 // --------------------------------------------------------------- RB interrupt initialization
    IOCB0 = 1;
    RBIF = 0;
    // RBIE = 1;
 // ---------------------------------------------------------------
    PEIE = 1;                   //Enable all peripheral interrupts 
    GIE = 1;                    //Globally Enable all interrupts 
}


void interrupt interrupt_handler(void) 
{
    if(CCP1IF == 1)
    {
        // if (period > 0)
//            stepcw();
        CCPR1 = period + CCPR1; 
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




void get_temperature(unsigned char TMP101_address, char *temp, char *temp_rem) {
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
    *temp = I2C_ReadByte(I2C_MORE);     //get upper 8 bits of temperature (integer portion)
    *temp_rem = I2C_ReadByte(I2C_LAST); //get lower 4 bits temperature (fractional portion)
    I2C_Stop();         // Generate stop condition
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