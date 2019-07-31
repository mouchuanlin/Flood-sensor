/* 
 * File:   battery_read.h
 * Author: Scott
 *
 * Created on January 24, 2018, 2:51 PM
 */

#ifndef BATTERY_READ_H
#define	BATTERY_READ_H

#include <xc.h>
#include <stdint.h>

extern void batteryTest(void);
extern bool isLowBattery(void);
static void startReadingFVR(void);
static bool isFVReadDone(void);
static uint16_t getVoltage(void);
extern bool isLowBattery(void);

uint8_t consecutiveLowReadings = 0;
uint16_t lowVoltageThr = 0x018E;//CF;

#endif	/* BATTERYREAD_H */