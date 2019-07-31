/* 
 * File:   floodmain.c
 * Author: Scott
 *
 * Created on March 12, 2019, 3:03 PM
 * 
 * 001->002: 04/17/19 (JV). Continuous transmit code for RF evaluation.
 * 002->003: 05/22/19 (JV). Enabled WDT.
 * 003->004: 05/31/19 (JV). DEMO ONLY. Use WOR for low current mode. Have to reset
 *                          the radio and re-configure registers for it to maintain
 *                          low power after transmission.
 * 004->005: 06/12/19 (JV). DEMO ONLY. 3 back-to-back pkts for every txmission. Tamper
 *                          restore for each battery change except on first power-up.
 * 005->006: 07/10/19 (JV). Ct's tx for RF testing. TEST ONLY.
 * 006->007: 07/17/19 (JV). Normal code with +6dB power level.
 * 007->008: 07/31/19 (JV). Normal code with 32bit sync.
 */

#include "cc1120_reg_config.h"
#include "config.h"
#include "transmit.h"
#include "io.h"
#include "spi.h"



#define VERSION "008"

#define ALREADY_BEEN_POWERED    0x02    // eeprom address

//serialNumberL = 0x75;
//serialNumberM = 0x72;
//serialNumberH = 0x62;

static void startup_blink(void);





void main() 
{
    init_flood();
    init_spi();
    register_config();
    startup_blink();
    
    CLRWDT();
    
//    if (eeprom_read(ALREADY_BEEN_POWERED) != 0x57)
//        eeprom_write(ALREADY_BEEN_POWERED, 0x57);
//    else
//        stillTampered = true;           // starts 20min restore timer
        
        
    init_spi();
    manual_calibration();
    CLRWDT();
//    __delay_us(10);
//    cmd_strobe(CC1120_STX);
    WDTCONbits.WDTPS = 0b01000;
    SWDTEN = 0;
    
    /* Verify we're not transmitting and shut down radio */
    while((cc1120_get_tx_status() & 0x70) != 0)
    {
        cmd_strobe(CC1120_SIDLE);
        __delay_ms(5);
    }
    cmd_strobe(CC1120_SWORRST);
    __delay_ms(5);
    cmd_strobe(CC1120_SWOR);
    SWDTEN = 1;
        
    while(1)
    {
        CLRWDT();
        /* Check if time for supervisory */
        check_for_supervisory();
        
        /* Check critical pins */
        checkTamper();
        checkAlarm();
        
//        if (ready_for_sleep())
//        {
            WPUC1 = 1;
            SLEEP();
            NOP();
            WPUC1 = 0;
//        }
        
        if (systemTicks >= 0xFFFFFF00) systemTicks = 0xFFFFFF00;
    }
}



void init_flood()
{
    CLRWDT();
    SWDTEN = 0;                     /* Turn off WDT for now */
    
    OSCCONbits.SPLLEN = 0b0;        /* Disable PLL */
    OSCCONbits.SCS = 0b00;          /* CLK Source is set by config word */
    OSCCONbits.IRCF = 0b1101;       /* Internal OSC = 4MHz */
    
    OSCTUNE = 0;
    
    INTCON = 0;                     /* Disable all interrupts for now */

    OPTION_REGbits.nWPUEN = 0b0;    /* WPU are enabled by individ. WPU latch vals */
    OPTION_REGbits.T0SE = 0b0;      /* Timer0 increments on low-to-high edge of RA4 (alarm pin) */
    OPTION_REGbits.T0CS = 0b0;      /* Timer0 clk source = F_osc/4 */
    OPTION_REGbits.PS = 0b111;      /* Prescaler rate = 1:256 */
    OPTION_REGbits.PSA = 0b0;       /* Prescaler assigned to timer0 */
    
    ANSELA = 0;
    ANSELC = 0;
    
    OPTION_REG = 0b00000111;  // WPU are enabled by individ. WPU latch vals.
    APFCON0 = 0b00000000;   // RC4->USART TX , RC5->USART RX
    
    TRISA = 0b00111010;
    WPUA = 0b00000110;      // RA2 on GPIO0, RA1 on JP2
    
    
    TRISC = 0b00010010;     // SDI on RC1
    WPUC = 0b00010000;
    CS = 1;
    LED = 0;
    
    
    assign_serial();
    
    // enable the fixed voltage reference spec
    // this is a known voltage that we use to measure against
    // the battery voltage. This code just sets up the
    // reference voltage
    FVRCONbits.ADFVR = 0b01; // Fixed voltage ref is 1x or (1.024 V)
    FVRCONbits.CDAFVR = 0b00; // not using DAC and CPS
    FVRCONbits.TSRNG = 0b0; // not using temp indicator
    FVRCONbits.TSEN = 0b0;  // temp sensor disabled
    
    INTCONbits.GIE = 1;
}



bool ready_for_sleep()
{
    cmd_strobe(CC1120_SIDLE);
    CLRWDT();
    while((cc1120_get_tx_status() & 0x70) != 0);
    cmd_strobe(CC1120_SPWD);     // Put CC115L to sleep
    __delay_us(50);
    return true;
}



static void startup_blink()
{
    LED_ON();
    __delay_ms(500);
    LED_OFF();
}