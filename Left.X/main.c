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

#define SENSEOR_A  0x90

void selectGame();
void enterData();
void game();

void initialize();
void testTemperature(unsigned char TMP101_address);
void configTemSensor(char TMP101_address);

void main(void) {
    initialize();
    mode = COMMAND;
    gameMode = DOUBLE;
    char temp_rem, temp;
    testTemperature(SENSEOR_A);
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
                game();
                break;
            case RESUT_DISPALY:
                result();
                break;
        }
    }
}

void game() {
    Note notes[LINE_WIDTH];
    signed char i, index, read;
    totalScore = 0; nCount = 0;

    testTemperature(SENSEOR_A);
    for (i = 0 ; i < LINE_WIDTH; i++){
        notes[i].keyEncoding = 0;
        notes[i].length = 2;
        notes[i].name[0] = ' ';
        notes[i].name[1] = ' ';
        notes[i].name[2] = '\0';
    }

    index = 0;
    read = END_SONG + 1;
    while(1) {
        if (read != END_SONG) {
            while (!hasInChar()) {
                if (mode != GAME)
                    return;
            }
            read = inChar();
            Note n = decode(read);
            notes[index] = n;
        }
        if (read == END_SONG) {
            notes[index].keyEncoding = 0;
            notes[index].length = 0; // to denote the end
            notes[index].name[0] = ' ';
            notes[index].name[1] = ' ';
            notes[index].name[2] = '\0';
        }

        index++;
        if (index == LINE_WIDTH)
            index = 0;

        lcd_clear();
        lcd_goto(0);
        for (i = LINE_WIDTH-1 ; i >= 0; i--){
            lcd_putch(notes[(i+index)%LINE_WIDTH].name[1]);
        }
        lcd_goto(0x40);
        for (i = LINE_WIDTH-1 ; i >= 0; i--){
            lcd_putch(notes[(i+index)%LINE_WIDTH].name[0]);
        }

        if (notes[index].length == 0)
            break;

        nCount++;

        totalScore += playNote(notes[index].keyEncoding, TIME_FACTOR * notes[index].length);
        updateNote(0xffff);
    }
    mode = RESUT_DISPALY;
}

void selectGame() {
    
}

void enterData() {
    char input, a, b;
    char commandBuf[3];
    while(1){
        while(!hasInChar());
        input = inChar();
        if (input == 0xff){ // start the game
            mode = GAME;
            return;
        }
        if (input == CMD_READ){
            while(!hasInChar());
            a = inChar();
            outChar(eeprom_read(a));
        }
        if (input == CMD_WRITE){
            while(!hasInChar());
            a = inChar();
            while(!hasInChar());
            b = inChar();
            eeprom_write(a, b);
        }
    }
}

void initialize() 
{
    
    general_init();
    
 // --------------------------------------------------------------- set this to 12-bit mode
    I2C_Initialize();
    configTemSensor(SENSEOR_A);

    PEIE = 1;                   //Enable all peripheral interrupts 
    GIE = 1;                    //Globally Enable all interrupts 
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
    if (temp > 50){
        lcd_clear();
        lcd_goto(0);
        lcd_puts("Power too hot");
        lcd_goto(0x40);
        lcd_puts("Stop playing, take some rest");
        outChar(0xff);// let the other PIC know.
        while(1);
    }
    
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
// delay

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
                        //the addressed TMP101.  (See Table 2 TMP101 Datasheet)
    I2C_SendByte(0x60); // Sets pointer register to 00 (configure register) for
    I2C_Stop();         // Generate stop condition

}



void interrupt interrupt_handler(void) 
{
    general_interrupt();

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
