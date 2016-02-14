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
#define SENSEOR_B  0x94

void initialize();
void get_temperature(unsigned char TMP101_address, char *temp, char *temp_rem);
void configTemSensor(char TMP101_address);
void updateMotorSpeed(char temp, char temp_rem);
void stepcw(void);
void stepccw(void);

signed char step_index = 0;
const unsigned char step_tbl[8] = {0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001, 0b0001}; // High-Torque Stepping Sequence (See Table)
int Sensor, period;

void main(void) {
    char temp, temp_rem;
    char sign_char, dig0_char, dig1_char, dig2_char, tenth_char, hundredth_char;
    initialize();
    Sensor = SENSEOR_A;
    period = 0;
    while (1) {
        get_temperature(Sensor, &temp, &temp_rem); // TMP101 with address 1001000
        updateMotorSpeed(temp, (temp_rem >> 4) & 0x0f );
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
        lcd_goto(0x40); // go the next line
        lcd_puts("Sensor: ");
        if (Sensor == SENSEOR_A) {
            lcd_putch('A');
        } else {
            lcd_putch('B');
        }
        DelayMs(200);
    }

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


void stepcw() {
    unsigned int count;
    step_index++;               // Step upward through the stepping sequence table
    if (step_index == 8) step_index = 0;    // Wrap when index gets up to 4

    PORTA = step_tbl[step_index];           // Send out the next step code from table
}

void stepccw() {
    unsigned int count;
    step_index--;               // Step downward through the stepping sequence table
    if (step_index == -1) step_index = 7;   // Wrap when index gets down to -1

    PORTA = step_tbl[step_index];           // Send out the next step code from table.
}


void updateMotorSpeed(char temp, char temp_rem) {
    int RPM;
    temp -= 25;
    if (temp & 0x80) {
        period = 0;
        return;
    }
    if (temp > 5 || ((temp == 5) && (temp_rem != 0) ))
        RPM = 240;
    else
        RPM = ((unsigned long)temp*16+temp_rem) * 180 /5 /16 + 60;
    period = 2000000 * 60 / 40 / RPM;
}    


void initialize() 
{
    
    ANSEL = 0; // disable analog input
    ANSELH = 0;
    nRBPU = 0; // enable inherent pull up resistor for PORTB
    TRISB = 0xff; // set PORTB as input
    TRISA = 0; // set PORTA as output, for debugging purpose
    
// -------------------------------------------------------------- comparator initialization from example
    TMR1GE=0;   TMR1ON = 1;         //Enable TIMER1 (See Fig. 6-1 TIMER1 Block Diagram in PIC16F887 Data Sheet)
    TMR1CS = 0;                     //Select internal clock whose frequency is Fosc/4, where Fosc = 4 MHz
    T1CKPS1 = 0; T1CKPS0 = 0;       //Set prescale to divide by 1 yielding a clock tick period of 1 microseconds

                            /*  From Section 6.12 of PIC16F887 Datasheet:
                                    bit 5-4 T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
                                    11 = 1:8 Prescale Value
                                    10 = 1:4 Prescale Value
                                    01 = 1:2 Prescale Value
                                    00 = 1:1 Prescale Value
                            */
    CCP1M3 = 0;CCP1M2 = 0;CCP1M1 = 1;CCP1M0 = 0;
                                //Set CCP1 mode for "Compare with toggle on CCP1 pin" 
                                //See REGISTER 11-1 datasheet
    // TRISC2 = 0;                 //Make CCP1 pin an output.
    CCP1IF = 0; 
    CCP1IE = 1;                 //Unmask (enable) Compare Interrupt from CCP1 (Interrupt when CCP1IF flag is set to 1)

    lcd_init();
 // --------------------------------------------------------------- set this to 12-bit mode
    I2C_Initialize();
    configTemSensor(SENSEOR_A);
    configTemSensor(SENSEOR_B);

 // --------------------------------------------------------------- RB interrupt initialization
    IOCB0 = 1;
    RBIF = 0;
    RBIE = 1;
 // ---------------------------------------------------------------
    PEIE = 1;                   //Enable all peripheral interrupts 
    GIE = 1;                    //Globally Enable all interrupts 
}


void interrupt interrupt_handler(void) 
{
    if(CCP1IF == 1)
    {
        if (period > 0)
            stepcw();
        CCPR1 = period + CCPR1; 
        CCP1IF = 0;     //Be sure to relax the CCP1 interrupt before returning.
    }
    RA5 = 0;
    if (RBIF == 1) {
        if (RB0 == 0) {
            if (Sensor == SENSEOR_A){
                RA5 = 1;
                Sensor = SENSEOR_B;
            }else
                Sensor = SENSEOR_A;
        }
        DelayMs(5);
        RBIF = 0;
    }
}
