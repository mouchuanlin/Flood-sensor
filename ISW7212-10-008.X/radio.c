
#include "config.h"
#include "cc1120_reg_config.h"
#include "battery_read.h"
#include "transmit.h"
#include "spi.h"
#include "io.h"


#define CRC_ENABLE          TRUE
#define CRC_INIT            0xFFFF
#define CRC16_POLY          0x8005

#define VCDAC_START_OFFSET 2
#define FS_VCO2_INDEX 0
#define FS_VCO4_INDEX 1
#define FS_CHP_INDEX 2


void register_config (void)
{
    uint8_t writeByte;
    unsigned short i;
    uint8_t t;
    
    /* Reset Radio */
    cmd_strobe(CC1120_SRES);
    
    /* Write register settings to radio */
    for ( i = 0; i < (sizeof(preferredSettings)/sizeof(registerSetting_t)); i ++)
    {
        writeByte = preferredSettings[i].data;
        radioRegWrite(preferredSettings[i].addr, &writeByte, 1);
        if (i == 0x2B)
        {
            cc1120_spi_read_reg(preferredSettings[i].addr, &t, 1);
            if (t == 0x56)
            {
                CLRWDT();
                __delay_ms(100);
                CLRWDT();
            }
        }
        else if (i == 0x2C)
        {
            cc1120_spi_read_reg(preferredSettings[i].addr, &t, 1);
            if (t == 0x36)
            {
                CLRWDT();
                __delay_ms(100);
                CLRWDT();
            }
        }
                
    }
}


/************* CRC16 *******************
*   Calculates the CRC-16 of a single byte the rf protocols.
*   @param  uint16_t crcVal  This iteration of CRC calculations will use this value to start out
*   @param  uint8_t data    This is the data that will generate the resulting CRC
*   @param  uint16_t poly   the polynomial to use for the CRC16
*   @return uint16_t crcVal  The resulting CRC
**************************************************/
uint16_t CRC_16( uint16_t crcVal, uint8_t data){
    bool carryBit = false;
    for( uint8_t bitMask = 0x80; bitMask != 0; bitMask >>= 1 )
    {
        carryBit = (bool)((crcVal & 0x8000) != 0);// !! is a convenient way to convert a value to a logical true of false value in C only
        crcVal <<= 1;
        if( ( bitMask & data ) != 0 )               //bit is a 1
        {
            if( !carryBit )
                crcVal ^= CRC16;
        }
        else                                      //bit is a 0
        {
            if( carryBit )
                crcVal ^= CRC16;
        }
    }
    return crcVal;
}

/************* CRC16_byteBuffer *******************
*   Implementing 16-bit CRC.
*   @param  uint8_t* byteBuffer  A pointer to an array of bytes that the crc will iterate through to calculate the CRC
*   @param  uint8_t bufferLength The number of bytes to use in the calculation.
*   @param  uint16_t poly   the polynomial to use for the CRC16
*   @return uint8_t The result of the CRC calculation
********************************************************/
uint16_t CS_rf_protocol_CRC16_byteBuffer(uint8_t* byteBuffer, uint8_t bufferLength){

    uint8_t i = 0;

    uint16_t crcResult = 0;

    for( ; i < bufferLength ; ++i )
    {
        crcResult = CRC_16( crcResult, byteBuffer[i] );
    }

    return crcResult;
}

extern void createPacket(uint8_t txBuffer[])
{
    uint32_t packet = *(uint32_t*)(&txPacket);
    txBuffer[0] = (uint8_t)((packet & 0xFF000000) >> 24);
    txBuffer[1] = (uint8_t)((packet & 0x00FF0000) >> 16);
    txBuffer[2] = (uint8_t)((packet & 0x0000FF00) >> 8);
    txBuffer[3] = (uint8_t)((packet & 0x000000FF) >> 0);
}



/*******************************************************************************
*   @fn         createPacket
*
*   @brief      This function is called before a packet is transmitted. It fills
*               the txBuffer with a packet consisting of a length byte, two
*               bytes packet counter and n random bytes.
*
*               The packet format is as follows:
*               |--------------------------------------------------------------|
*               |           |           |           |         |       |        |
*               | Dev  type | pktCount1 | pktCount0 | rndData |.......| rndData|
*               |           |           |           |         |       |        |
*               |--------------------------------------------------------------|
*                txBuffer[0] txBuffer[1] txBuffer[2]            txBuffer[PKTLEN]
*
*   @param       Pointer to start of txBuffer
*
*   @return      none
*/
extern void create_ack_packet(uint8_t *txBuffer) 
{
    *txBuffer = 0x06;
}


//void writePreamble(uint16_t preambleTime)
//{   
//    trxCmdStrobe(CC1120_STX);
//    __delay_ms(100);
//}


/*******************************************************************************
 * @fn          cc112xSpiReadRxFifo
 *
 * @brief       Reads RX FIFO values to pData array
 *
 * input parameters
 *
 * @param       *pData - pointer to data array where RX FIFO bytes are saved
 * @param       len    - number of bytes to read from the RX FIFO
 *
 * output parameters
 *
 * @return      rfStatus_t
 */
rfStatus_t cc1120_spi_read_rx_fifo(uint8_t *pData, uint8_t len)
{
  rfStatus_t rc;
  rc = trx8BitRegAccess(0x00,CC1120_BURST_RXFIFO, pData, len);
  return (rc);
}


/*******************************************************************************
*   @fn         calibrateRcOsc
*
*   @brief      Calibrates the RC oscillator used for the eWOR timer. When this
*               function is called, WOR_CFG0.RC_PD must be 0
*
*   @param      none
*
*   @return     none
*/
void calibrate_rc_osc(void) 
{
    uint8_t t;

    // Read current register value
    cc1120_spi_read_reg(CC1120_WOR_CFG0, &t,1);

    // Mask register bit fields and write new values
    t = (uint8_t)((t & 0xF9) | (0x02 << 1));

    // Write new register value
    radioRegWrite(CC1120_WOR_CFG0, &t,1);

    // Strobe IDLE to calibrate the RCOSC
    cmd_strobe(CC1120_SIDLE);

    while((cc1120_get_tx_status() & 0x70) != 0);
    // Disable RC calibration
    t = (uint8_t)((temp & 0xF9) | (0x00 << 1));
    radioRegWrite(CC1120_WOR_CFG0, &t, 1);
}

/*******************************************************************************
*   @fn         manual_calibration
*
*   @brief      Calibrates radio according to CC112x errata
*
*   @param      none
*
*   @return     none
*/
void manual_calibration(void) {

    unsigned char original_fs_cal2;
    unsigned char calResults_for_vcdac_start_high[3];
    unsigned char calResults_for_vcdac_start_mid[3];
    unsigned char marcstate;
    unsigned char writeByte;

    // 1) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    writeByte = 0x00;
    radioRegWrite(CC1120_FS_VCO2, &writeByte, 1);

    // 2) Start with high VCDAC (original VCDAC_START + 2):
    cc1120_spi_read_reg(CC1120_FS_CAL2, &original_fs_cal2, 1);
    writeByte = (unsigned char)(original_fs_cal2 + VCDAC_START_OFFSET);
    radioRegWrite(CC1120_FS_CAL2, &writeByte, 1);

    // 3) Calibrate and wait for calibration to be done
    //   (radio back in IDLE state)
    cmd_strobe(CC1120_SCAL);

    do {
        cc1120_spi_read_reg(CC1120_MARCSTATE, &marcstate, 1);
    } while (marcstate != 0x41);

    // 4) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained with 
    //    high VCDAC_START value
    cc1120_spi_read_reg(CC1120_FS_VCO2,
                     &calResults_for_vcdac_start_high[FS_VCO2_INDEX], 1);
    cc1120_spi_read_reg(CC1120_FS_VCO4,
                     &calResults_for_vcdac_start_high[FS_VCO4_INDEX], 1);
    cc1120_spi_read_reg(CC1120_FS_CHP,
                     &calResults_for_vcdac_start_high[FS_CHP_INDEX], 1);

    // 5) Set VCO cap-array to 0 (FS_VCO2 = 0x00)
    writeByte = 0x00;
    radioRegWrite(CC1120_FS_VCO2, &writeByte, 1);

    // 6) Continue with mid VCDAC (original VCDAC_START):
    writeByte = original_fs_cal2;
    radioRegWrite(CC1120_FS_CAL2, &writeByte, 1);

    // 7) Calibrate and wait for calibration to be done
    //   (radio back in IDLE state)
    cmd_strobe(CC1120_SCAL);

    do {
        cc1120_spi_read_reg(CC1120_MARCSTATE, &marcstate, 1);
    } while (marcstate != 0x41);

    // 8) Read FS_VCO2, FS_VCO4 and FS_CHP register obtained 
    //    with mid VCDAC_START value
    cc1120_spi_read_reg(CC1120_FS_VCO2, 
                     &calResults_for_vcdac_start_mid[FS_VCO2_INDEX], 1);
    cc1120_spi_read_reg(CC1120_FS_VCO4,
                     &calResults_for_vcdac_start_mid[FS_VCO4_INDEX], 1);
    cc1120_spi_read_reg(CC1120_FS_CHP,
                     &calResults_for_vcdac_start_mid[FS_CHP_INDEX], 1);

    // 9) Write back highest FS_VCO2 and corresponding FS_VCO
    //    and FS_CHP result
    if (calResults_for_vcdac_start_high[FS_VCO2_INDEX] >
        calResults_for_vcdac_start_mid[FS_VCO2_INDEX]) {
        writeByte = calResults_for_vcdac_start_high[FS_VCO2_INDEX];
        radioRegWrite(CC1120_FS_VCO2, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_high[FS_VCO4_INDEX];
        radioRegWrite(CC1120_FS_VCO4, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_high[FS_CHP_INDEX];
        radioRegWrite(CC1120_FS_CHP, &writeByte, 1);
    } else {
        writeByte = calResults_for_vcdac_start_mid[FS_VCO2_INDEX];
        radioRegWrite(CC1120_FS_VCO2, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_mid[FS_VCO4_INDEX];
        radioRegWrite(CC1120_FS_VCO4, &writeByte, 1);
        writeByte = calResults_for_vcdac_start_mid[FS_CHP_INDEX];
        radioRegWrite(CC1120_FS_CHP, &writeByte, 1);
    }
}



void append_crc(uint8_t txBuf[])
{
    /* Append CRC */
    uint16_t tempBuf = CS_rf_protocol_CRC16_byteBuffer(txBuf, 4);
    txBuf[4] = (uint8_t)((tempBuf & 0xFF00) >> 8);                       // High byte of CRC
    txBuf[5] = (uint8_t)((tempBuf & 0x00FF) >> 0);                       // Low byte of CRC
}