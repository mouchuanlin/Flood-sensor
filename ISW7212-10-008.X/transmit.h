/* 
 * File:   transmit.h
 * Author: Scott
 *
 * Created on January 24, 2018, 3:05 PM
 */

#ifndef TRANSMIT_H
#define	TRANSMIT_H

#include <stdint.h>

enum TransmitType {alarm, test, tamper, supervisory, lowBatt, err, restore, temp};

extern void createPacket(uint8_t *txBuffer);
extern void burstTX(uint8_t txBuffer[], uint8_t len, bool isAlarm);
void writePreamble(void);
void transmit (enum TransmitType txType);
void writePacket(uint8_t txBuffer[], uint8_t len);
void delaySlots(uint8_t numSlotsToWait);
void assign_serial(void);

uint16_t CRC_16( uint16_t crcVal, uint8_t data);
uint16_t CS_rf_protocol_CRC16_byteBuffer(uint8_t* byteBuffer, uint8_t bufferLength);
uint16_t random(uint8_t seed, uint8_t option);



#endif	/* TRANSMIT_H */