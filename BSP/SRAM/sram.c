#include "./BSP/SRAM/sram.h"
#include "./SYSTEM/usart/usart.h"


/**
 * @brief   Initialize external SRAM (FSMC Bank3, NE3 at PG10)
 * @param   None
 * @retval  None
 */
void sram_init(void)
{
    SRAM_HandleTypeDef sram_handle;
    FSMC_NORSRAM_TimingTypeDef fsmc_readwritetim;

    /* CubeMX FSMC init (fsmc.c) already configures all FSMC GPIO pins and clocks.
     * This function only configures the FSMC timing for SRAM (Bank3/NE3). */

    sram_handle.Instance = FSMC_NORSRAM_DEVICE;
    sram_handle.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

    sram_handle.Init.NSBank = (SRAM_FSMC_NEX == 1) ? FSMC_NORSRAM_BANK1 :
                                 (SRAM_FSMC_NEX == 2) ? FSMC_NORSRAM_BANK2 :
                                 (SRAM_FSMC_NEX == 3) ? FSMC_NORSRAM_BANK3 :
                                                        FSMC_NORSRAM_BANK4;
    sram_handle.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    sram_handle.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    sram_handle.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    sram_handle.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    sram_handle.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    sram_handle.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    sram_handle.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    sram_handle.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    sram_handle.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
    sram_handle.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    sram_handle.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

    /* FSMC timing: ADDSET=2, DATAST=8, Mode A */
    fsmc_readwritetim.AddressSetupTime = 0x02;
    fsmc_readwritetim.AddressHoldTime = 0x00;
    fsmc_readwritetim.DataSetupTime = 0x08;
    fsmc_readwritetim.BusTurnAroundDuration = 0x00;
    fsmc_readwritetim.AccessMode = FSMC_ACCESS_MODE_A;
    HAL_SRAM_Init(&sram_handle, &fsmc_readwritetim, &fsmc_readwritetim);
}

/**
 * @brief   Write data to SRAM at specified address
 * @param   pbuf    : data buffer
 * @param   addr    : start address (32bit)
 * @param   datalen : byte count (32bit)
 * @retval  None
 */
void sram_write(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    for (; datalen != 0; datalen--)
    {
        *(volatile uint8_t *)(SRAM_BASE_ADDR + addr) = *pbuf;
        addr++;
        pbuf++;
    }
}

/**
 * @brief   Read data from SRAM at specified address
 * @param   pbuf    : data buffer
 * @param   addr    : start address (32bit)
 * @param   datalen : byte count (32bit)
 * @retval  None
 */
void sram_read(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    for (; datalen != 0; datalen--)
    {
        *pbuf++ = *(volatile uint8_t *)(SRAM_BASE_ADDR + addr);
        addr++;
    }
}

/******************* test functions **********************************/

/**
 * @brief   Write 1 byte to SRAM test address
 * @param   addr : address
 * @param   data : byte to write
 * @retval  None
 */
void sram_test_write(uint32_t addr, uint8_t data)
{
    sram_write(&data, addr, 1);
}

/**
 * @brief   Read 1 byte from SRAM test address
 * @param   addr : address
 * @retval  data read
 */
uint8_t sram_test_read(uint32_t addr)
{
    uint8_t data;
    sram_read(&data, addr, 1);
    return data;
}
