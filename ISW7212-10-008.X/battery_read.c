
#include "config.h"
#include "battery_read.h"


extern bool isLowBattery()
{   /* Returns 1 for low, 0 for OK */
    return (bool)((consecutiveLowReadings >= 3 ? 1 : 0) || low_bat_flag);       // lowReadings count is for smoke daughterboard, low batt flag is for motherboard
}

extern void batteryTest()
{
    if (isLowBattery())
        return;                       // Once battery is detected low, it stays low
    
    startReadingFVR();
    
    while (!isFVReadDone());  // Spin for ~100us
    uint16_t voltage = (uint16_t)getVoltage();
    ADCON1 = 0;                         
    ADCON0bits.ADON = 0;                // Disable Analog
    
    FVRCONbits.FVREN = 0;               // Disable FVR
    if (voltage >= lowVoltageThr)
    {
        if (consecutiveLowReadings < 3)
            consecutiveLowReadings ++;
        
        else
            consecutiveLowReadings = 0;
    }
    return;
}

static void startReadingFVR()
{
    FVRCONbits.FVREN = 0b1;     // enable fvr
    while(!FVRCONbits.FVRRDY);  // spin until ready
    
    ADCON1bits.ADPREF = 0b00;   // read fixed voltage reg
    ADCON1bits.ADCS = 0b100;    // Fosc/4 for a 1Mhz clock is in the recommended range 
                                // See table 16-1 on page 154
    ADCON1bits.ADFM = 0b1;      // result is right justified in ADRESH
    ADCON0bits.CHS = 0b11111;   //FVR Buffer 1   
    ADCON0bits.ADON = 1;        // enable analog
    
    
    __delay_us(20); // this setup delay is necessary for the ADC hardware
   
    // start read
    ADCON0bits.GO = 1; 
}

static bool isFVReadDone()
{
    return (bool)!ADCON0bits.GO_nDONE;
}

static uint16_t getVoltage()
{
    uint8_t lo = ADRESLbits.ADRESL;
    uint8_t hi = (uint8_t)(ADRESHbits.ADRESH & 0x03);
    uint16_t result = (uint16_t)((hi << 8) | (lo & 0xFC));  // We only care about 8 MSB in this 10-bit measurement
    return result;
}