/* 
 * File:   config.h
 * Author: Scott
 *
 * Created on March 12, 2019, 3:04 PM
 */

#ifndef CONFIG_H
#define	CONFIG_H

// CONFIG1
#pragma config FOSC = INTOSC       // Oscillator Selection (ECH, External Clock, High Power Mode (4-32 MHz): device clock supplied to CLKIN pin)
#pragma config WDTE = SWDTEN        // Watchdog Timer Enable (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = ON          // Flash Program Memory Code Protection (Program memory code protection is enabled)
#pragma config CPD = ON         // Data Memory Code Protection (Data memory code protection is enabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


/*******************************************
 * INCLUDES
 ******************************************/
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#define _XTAL_FREQ  4000000
#define BAUD        9600
#define SDI_PIN     TRISC1
#define SDO_PIN     TRISC2
//#define JUMPER    RC4

#define WATCHDOG_SLEEP_256ms    0b01000
#define WATCHDOG_SLEEP_512ms    0b01001
#define WATCHDOG_SLEEP_1S       0b01010
#define WATCHDOG_SLEEP_2S       0b01011
#define WATCHDOG_SLEEP_4S       0b01100
#define WATCHDOG_SLEEP_8S       0b01101
#define WATCHDOG_SLEEP_16S      0b01110
#define WATCHDOG_SLEEP_32S      0b01111
#define WATCHDOG_SLEEP_64S      0b10000
#define WATCHDOG_SLEEP_128S     0b10001
#define WATCHDOG_SLEEP_256S     0b10010


#define LED             LATCbits.LATC5
#define GPIO0           RA2
/* UNUSED */
//#define GPIO2           RC1
//#define GPIO3           RB7

#define nTEST_JUMPER    RA1
#define ALARM           RA5
#define TAMPER          RA4

#define RLED_INPUT      TRISC5
#define GPIO0_INPUT     TRISA2
#define ALARM_INPUT     TRISA5
#define TAMPER_INPUT    TRISA4
#define JUMPER_INPUT    TRISA1
#define nRESET_INPUT    TRISC4

#define JUMPER_PU       WPUA1
#define nRESET_PU       WPUC4

#define ALARM_PIN_FLAG  IOCAF5
#define TAMP_PIN_FLAG   IOCAF4

#define IDH             0x00
#define IDM             0x02
#define IDL             0x04

#define _20MIN          9372
#define _5MIN           2343


#define SupervisoryTimer        675000//1 day supervisory, based on 128ms WDT
#define CRC16                   0x8005
#define DEVICE_TYPE             0b0110

/*******************************************
 * CUSTOM FUNCTION DEFINES
 ******************************************/
#define EN_TEST_INTH(void)          IOCAP1=1        // Enable interrupt on TEST pin change low-to-high
#define DISABLE_TEST_INTH(void)     IOCAP1=0        // Disable above interrupt
#define EN_ALARM_INTH(void)         IOCAP2=1        // Enable interrupt on ALARM pin change lo-hi
#define DISABLE_ALARM_INTH(void)    IOCAP2=0        // Disable above interrupt
#define EN_TAMP_INTH(void)          IOCAP4=1        // Enable interrupt on ERROR pin change lo-hi
#define DISABLE_TAMP_INTH(void)     IOCAP4=0        // Disable above interrupt
#define EN_TAMP_INTL(void)          IOCAN4=1
#define DISABLE_TAMP_INTL(void)     IOCAN4=0
#define LED_ON()                LED=1
#define LED_OFF()               LED=0


/*******************************************
 * STRUCTS
 ******************************************/
struct PacketData {                     // in LSB -> MSB order
    unsigned TRIGGER            : 1;
    unsigned TAMPER_p           : 1;
    unsigned TEST               : 1;
    unsigned unused1            : 1;
    unsigned LOW_BATT           : 1;
    unsigned unused2            : 1;
    unsigned ERR                : 1;
//    unsigned TEST               : 1;    // OPTIONAL
    unsigned SUPERVISORY        : 1;
    unsigned SERIAL_IDL         : 8;
    unsigned SERIAL_IDM         : 8;
    unsigned SERIAL_IDH         : 4;
    unsigned TYPE               : 4;    // Alarm type (flood/smoke/glass/etc.)
};



/*******************************************
 * PROTOTYPES
 ******************************************/
void init_flood(void);
void register_config(void);
void cfg_interrupts(void);
extern uint8_t is_low_battery(void);
void check_for_supervisory(void);
void store_low_batt(void);
void assign_serial(void);
void append_crc(uint8_t txBuf[]);
bool ready_for_sleep(void);


/*******************************************
 * VARIABLES
 ******************************************/
struct PacketData txPacket;
unsigned short packetCounter;
uint8_t lowBattCount = 0;
uint16_t preambleTimespan = 6000;
uint32_t systemTicks = 0;
bool waitingFor20s = false;
bool low_bat_flag = false;


//extern uint8_t serialNumberL __at(0x0020);// = 0x75;
//extern uint8_t serialNumberM __at(0x0021);// = 0x72;
//extern uint8_t serialNumberH __at(0x0022);// = 0x62;

const char serialNum[] __at(0x0020) = {0x75,0x72,0x62};
#endif	/* CONFIG_H */