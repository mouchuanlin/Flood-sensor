/* 
 * File:   spi.h
 * Author: Scott
 *
 * Created on March 12, 2019, 3:14 PM
 */

#ifndef SPI_H
#define	SPI_H

#include <stdbool.h>

#define RADIO_BURST_ACCESS   0x40        // Command address byte mask for busrst register access
#define RADIO_SINGLE_ACCESS  0x00
#define RADIO_READ_ACCESS    0x80
#define RADIO_WRITE_ACCESS   0x00


/* Bit fields in the chip status byte */
#define STATUS_CHIP_RDYn_BM             0x80
#define STATUS_STATE_BM                 0x70
#define STATUS_FIFO_BYTES_AVAILABLE_BM  0x0F


#define MISO        RC1
#define MOSI        RC2
#define CS          RC3

#define TRXEM_SPI_BEGIN()   CS=0     
#define TRXEM_SPI_END()     CS=1


#define NUM_TIMESTAMP_PACKETS           1800
#define PKTLEN                          10


/*****************************************************
 * TYPEDEFS
 ****************************************************/
typedef unsigned char rfStatus_t;


/*****************************************************
 * PROTOTYPES
 ****************************************************/
rfStatus_t radioRegWrite(uint16_t addr, uint8_t *data, uint8_t len);
rfStatus_t cc1120_spi_read_reg(uint16_t addr, uint8_t *data, uint8_t len);
rfStatus_t trx8BitRegAccess(uint8_t accessType, uint8_t addrByte, uint8_t *pData, uint8_t len);
rfStatus_t trx16BitRegAccess(uint8_t accessType, uint8_t extAddr, uint8_t regAddr, uint8_t *data, uint8_t len);
rfStatus_t cc1120_spi_write_tx_fifo(unsigned char *pData, unsigned char len);
rfStatus_t cc1120_spi_read_rx_fifo(uint8_t *pData, uint8_t len);
rfStatus_t cc1120_get_tx_status(void);
rfStatus_t cmd_strobe(uint8_t cmd);

uint16_t calc_crc(uint8_t crcData, uint16_t crcReg);
uint8_t spi_write(uint8_t data);
void trxReadWriteBurstSingle(uint8_t addr, uint8_t *data, uint8_t len);
extern void create_ack_packet(unsigned char txBuffer[]);
extern void run_tx(void);
extern bool received_data(uint8_t rxBuffer[], uint8_t* rxBytes, uint8_t* marcStatus);
void init_spi(void);
void manual_calibration(void);
void calibrate_rc_osc(void);

#endif	/* SPI_H */