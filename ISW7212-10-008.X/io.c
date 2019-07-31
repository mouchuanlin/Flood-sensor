
#include "config.h"
#include "transmit.h"
#include "io.h"




bool sentErr = false;
uint16_t tamperOpenTimerCnt = 0;
bool powerOnTamper = true;
uint16_t cnt = 100;

void checkAlarm() 
{
    /* Once it goes off, checks alarm and re-sends alarm signal with every supervisory */
    if (ALARM && !stillTriggered && nTEST_JUMPER)
    {
        CLRWDT();
        __delay_ms(50);
        if (ALARM && nTEST_JUMPER)
        {
            transmit(alarm);
            stillTriggered = true;
            
            // assume 6 seconds * 2 * 4
            systemTicks += 6 * 50 * 2 * 4;
        }
    }
    else if (!ALARM && stillTriggered && nTEST_JUMPER)
        checkToResetAlarm();
    else if (ALARM && !nTEST_JUMPER)
    {
        CLRWDT();
        __delay_ms(50);
        if (ALARM && !nTEST_JUMPER)
        {
            transmit(test);
            // assume 18s total
            systemTicks += 6 * 50 * 3;
        }
    }
}


void checkTamper()
{
    if (powerOnTamper && !TAMPER)       // ignore tamper on boot
    {
        CLRWDT();
//        if (!TAMPER && cnt-- > 0)       // buffer the tamper switch on initial boot by 10s
//        {                               // based on 128ms WDT
//            __delay_ms(1);
//            CLRWDT();
//        }
        cnt--;                  // buffer tamper sw on initial boot by 10s
                                // based on 128ms WDT
        if (!TAMPER && cnt <= 0)
        {
            powerOnTamper = false;
            IOCAPbits.IOCAP4 = 1;
            INTCONbits.IOCIE = 1;
            INTCONbits.IOCIF = 0;
        }
    }
    else if (powerOnTamper && TAMPER)
    {
        cnt = 100;
        IOCAPbits.IOCAP4 = 0;
        INTCONbits.IOCIE = 0;
        INTCONbits.IOCIF = 0;
        tampIntOccurred = false;
    }
    
    
    if(TAMPER && !stillTampered && !powerOnTamper)
    {
        /* Checks tamper and re-sends tamper signal with every supervisory */
        CLRWDT();
        __delay_ms(50);
        CLRWDT();
        
        if (TAMPER)
        {
//            blinkLED();
            transmit(tamper);
            stillTampered = true;
            _20minTimerCnt = 0;
        }
    }
    else if (TAMPER && stillTampered)
    {
        __delay_ms(30);
        if (TAMPER)             // then check the port
        {
            _20minTimerCnt = 0;
            tamperOpenTimerCnt++;
            if (tamperOpenTimerCnt == 1)
                blinkLED();
        }
        else
            checkToResetTamper();   // this allows the unit to restore tamper even with low batt
    }

    else if (!TAMPER && stillTampered)
        checkToResetTamper();
}


void checkToResetAlarm()
{
    if (stillTriggered && !ALARM)
    {
        CLRWDT();
        __delay_ms(50);
        CLRWDT();
    
        if (!ALARM)
            stillTriggered = false;
    }
}


void checkToResetTamper()
{
//    if (_20minTimerCnt % 30 == 0)
//        LED_OFF();
//    else if (_20minTimerCnt % 30 == 23)     // slow status blink
//        LED_ON();
    if (stillTampered && !TAMPER)
    {
        CLRWDT();
        _20minTimerCnt++;
        tamperOpenTimerCnt = 0;
        if (_20minTimerCnt == 1)        // blink LED on tamper close
            blinkLED();
        if (_20minTimerCnt >= _20MIN)
        {
            LED_OFF();
            _20minTimerCnt = 0;     // reset tamper after 20min of being "restored"
            stillTampered = false;
            transmit(restore);
        }
    
    }
}


void blinkLED()
{
    uint8_t tick = 1;
    CLRWDT();
    if (low_bat_flag)
        tick = 3;
    for (uint8_t i = 0; i < tick; i++)
    {
        LED_ON();
        CLRWDT();
        __delay_ms(100);
        CLRWDT();
        __delay_ms(100);
        LED_OFF();
        CLRWDT();
        __delay_ms(100);
        CLRWDT();
        __delay_ms(100);
        CLRWDT();
    }
}


void check_for_supervisory()
{
    if (systemTicks >= SupervisoryTimer)
    {
        systemTicks = 0;
        // don't transmit message if not in supervisory mode
        if (isSupervisoryMode)
            transmit(supervisory);
    }
}


void assign_serial()         // Assigns Serial ID and device type for every transmission
{
    txPacket.TYPE = DEVICE_TYPE;         /* Smoke has device type ID 0x8 */
    txPacket.SERIAL_IDH = serialNum[2] & 0x0F;
    txPacket.SERIAL_IDM = serialNum[1];
    txPacket.SERIAL_IDL = serialNum[0];
}