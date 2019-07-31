/* 
 * File:   IO.h
 * Author: Scott
 *
 * Created on January 7, 2019, 1:37 PM
 */

#ifndef IO_H
#define	IO_H

#define __20seconds         108         // Count for timer1 to amount to 20s
#define _43s                250//300//290//316//320         // Based on measured 134ms, minus some overhead
#define REPEAT_CNT          5           // Number times a low batt signal is 
                                        // detected from smoke before transmitting it

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>


void checkAlarm(void);
void checkTamper(void);
void checkTest(void);
void checkToResetAlarm(void);
void checkToResetTamper(void);
void checkToResetTest(void);
void blinkLED(void);

uint8_t saveINTCONbits;
uint8_t tampIntCnt = 0, errRepeatCnt = 0, consecutiveLowBattCnt = 0;
uint16_t _43sTmrCnt = 0;
uint16_t _20minTimerCnt = 0;

bool stillTampered;
bool stillTriggered;
bool stillTesting;
bool tamperNeedsDebounce = false;
bool testTriggered = false;
bool alarmNeedsDebounce = false;
bool isSupervisoryMode = true;
bool isTestMode = false;
bool blink = true;                   // Blink LED @ 100ms duty during transmit
bool tampIntOccurred = false;
bool _43sTmrOn = false;
bool _43sTmrDone = false;
    

#endif	/* IO_H */