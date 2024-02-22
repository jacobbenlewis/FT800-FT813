/*
@file    EVE_target_Tricore_Tasking.h
@brief   target specific includes, definitions and functions
@version 5.0
@date    2023-09-02
@author  Rudolph Riedel

@section LICENSE

MIT License

Copyright (c) 2016-2023 Rudolph Riedel

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

@section History

5.0
- extracted from EVE_target.h

*/

#ifndef EVE_TARGET_TRICORE_H
#define EVE_TARGET_TRICORE_H

#if !defined (ARDUINO)
#define __TASKING__   // TODO: REMOVE!!!!!!!
#if defined(__TASKING__)

#include <stdint.h>

#include "IfxPort.h"
#include "Bsp.h"


#define EVE_PD  &MODULE_P14,4
#define EVE_CS  &MODULE_P15,2

#define DELAY_MS(ms) (waitTime(IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER, ms)))

static inline void EVE_pdn_set(void)
{
    IfxPort_setPinLow(EVE_PD);
}

static inline void EVE_pdn_clear(void)
{
    IfxPort_setPinHigh(EVE_PD);
}

static inline void EVE_cs_set(void)
{
    IfxPort_setPinLow(EVE_CS);
}

static inline void EVE_cs_clear(void)
{
    IfxPort_setPinHigh(EVE_CS);
}

static inline void spi_transmit(uint8_t data)
{
    (void) data;
    __nop();
    // Not supported with IfxQspi driver out of the box!!  Maybe with MCAL?
//    SPI_ReceiveByte(data);
}

static inline void spi_transmit_32(uint32_t data)
{
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=(uint8_t)(data & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=(uint8_t)((data >> 8) & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=(uint8_t)((data >> 16) & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[3]=(uint8_t)((data >> 24) & 0xff);
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 4);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}
}

/* spi_transmit_burst() is only used for cmd-FIFO commands */
/* so it *always* has to transfer 4 bytes */
static inline void spi_transmit_burst(uint32_t data)
{
    spi_transmit_32(data);
}

static inline uint8_t spi_receive(uint8_t data)
{
    (void) data;
    return (0);
    // Not supported with IfxQspi driver out of the box!!  Maybe with MCAL?
//    return (SPI_ReceiveByte(data));
}

static inline uint8_t fetch_flash_byte(const uint8_t *p_data)
{
    return (*p_data);
}

static inline void EVE_start_burst()
{
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=0xB0U; /* high-byte of REG_CMDB_WRITE + MEM_WRITE */
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=0x25U; /* middle-byte of REG_CMDB_WRITE */
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=0x78U; /* low-byte of REG_CMDB_WRITE */
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 3);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}
}

static inline void EVE_host_command(uint8_t HCMD, uint8_t param)
{
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=HCMD;
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=param;
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=0x00;
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 3);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}
}

static inline void spi_wr32(uint32_t address, uint32_t parameter)
{
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=((address >> 16) | 0x80); // RAM_REG = 0x302000 and high bit is set - result always 0xB0;
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=(uint8_t)(address >> 8); // Next byte of the register address;
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=(uint8_t)address; // Low byte of register address - usually just the 1 byte offset;
    g_qspi.spiBuffers.spiMasterTxBuffer[3]=(uint8_t)(parameter & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[4]=(uint8_t)((parameter >> 8) & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[5]=(uint8_t)((parameter >> 16) & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[6]=(uint8_t)((parameter >> 24) & 0xff);
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 7);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}
}

static inline void spi_wr16(uint32_t address, uint16_t parameter)
{
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=((address >> 16) | 0x80); // RAM_REG = 0x302000 and high bit is set - result always 0xB0;
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=(uint8_t)(address >> 8); // Next byte of the register address;
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=(uint8_t)address; // Low byte of register address - usually just the 1 byte offset;
    g_qspi.spiBuffers.spiMasterTxBuffer[3]=(uint8_t)(parameter & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[4]=(uint8_t)((parameter >> 8) & 0xff);
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 5);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}
}

static inline void spi_wr8(uint32_t address, uint8_t parameter)
{
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=((address >> 16) | 0x80); // RAM_REG = 0x302000 and high bit is set - result always 0xB0;
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=(uint8_t)(address >> 8); // Next byte of the register address;
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=(uint8_t)address; // Low byte of register address - usually just the 1 byte offset;
    g_qspi.spiBuffers.spiMasterTxBuffer[3]=(uint8_t)(parameter & 0xff);
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 4);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}
}

static inline uint32_t spi_rd32(uint32_t address)
{
    uint32_t Data32;
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=(address >> 16) & 0x3F; // RAM_REG = 0x302000 and high bit is set - result always 0xB0;
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=(address >> 8) & 0xff; // Next byte of the register address;
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=address & 0xff; // Low byte of register address - usually just the 1 byte offset;
    g_qspi.spiBuffers.spiMasterTxBuffer[3]=0x00;
    g_qspi.spiBuffers.spiMasterTxBuffer[4]=0x00;
    g_qspi.spiBuffers.spiMasterTxBuffer[5]=0x00;
    g_qspi.spiBuffers.spiMasterTxBuffer[6]=0x00;
    g_qspi.spiBuffers.spiMasterTxBuffer[7]=0x00;
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 8);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}

    Data32 = g_qspi.spiBuffers.spiMasterRxBuffer[4] +
            ((uint32_t)g_qspi.spiBuffers.spiMasterRxBuffer[5] << 8) +
            ((uint32_t)g_qspi.spiBuffers.spiMasterRxBuffer[6] << 16) +
            ((uint32_t)g_qspi.spiBuffers.spiMasterRxBuffer[7] << 24);

    return (Data32);
}

static inline uint16_t spi_rd16(uint32_t address)
{
    uint16_t Data16;
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=((address >> 16) & 0x3F); // RAM_REG = 0x302000 and high bit is set - result always 0xB0;
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=((address >> 8) & 0xff); // Next byte of the register address;
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=address & 0xff; // Low byte of register address - usually just the 1 byte offset;
    g_qspi.spiBuffers.spiMasterTxBuffer[3]=0x00;
    g_qspi.spiBuffers.spiMasterTxBuffer[4]=0x00;
    g_qspi.spiBuffers.spiMasterTxBuffer[5]=0x00;
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 6);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}
    Data16 = g_qspi.spiBuffers.spiMasterRxBuffer[4] +
           ((uint32_t)g_qspi.spiBuffers.spiMasterRxBuffer[5] << 8);

    return (Data16);
}

static inline uint8_t spi_rd8(uint32_t address)
{
    uint8_t Data8;
    g_qspi.spiBuffers.spiMasterTxBuffer[0]=((address >> 16) & 0x3F);
    g_qspi.spiBuffers.spiMasterTxBuffer[1]=((address >> 8) & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[2]=(address & 0xff);
    g_qspi.spiBuffers.spiMasterTxBuffer[3]=0x00;
    g_qspi.spiBuffers.spiMasterTxBuffer[4]=0x00;
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], &g_qspi.spiBuffers.spiMasterRxBuffer[0], 5);
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy) {}
    Data8 = g_qspi.spiBuffers.spiMasterRxBuffer[4];
    return (Data8);
}

#endif /* __TASKING__ */
#endif /* !Arduino */

#endif /* EVE_TARGET_TRICORE_H */
