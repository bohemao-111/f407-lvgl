
#include "stdlib.h"
#include <stdio.h>
#include "lcd.h"
#include "lcdfont.h"
#include "./SYSTEM/usart/usart.h"


/* lcd_ex.c contains register initialization for various LCD driver ICs.
 * It is included here directly since it's only used by lcd.c.
 */
#include "./BSP/LCD/lcd_ex.c"


SRAM_HandleTypeDef g_sram_handle;   /* SRAM handle (for LCD FSMC) */

/* LCD brush color and background color */
uint32_t g_point_color = 0xF800;    /* default: red */
uint32_t g_back_color  = 0xFFFF;    /* default: white */

/* LCD parameters */
_lcd_dev lcddev;

/**
 * @brief   LCD write data
 * @param   data: 16-bit data
 */
void lcd_wr_data(volatile uint16_t data)
{
    data = data;            /* prevent optimization with -O2 */
    LCD->LCD_RAM = data;
}

/**
 * @brief   LCD write register index
 * @param   regno: register number/index
 */
void lcd_wr_regno(volatile uint16_t regno)
{
    regno = regno;          /* prevent optimization with -O2 */
    LCD->LCD_REG = regno;
}

/**
 * @brief   LCD write register with value
 * @param   regno: register number
 * @param   data:  value to write
 */
void lcd_write_reg(uint16_t regno, uint16_t data)
{
    LCD->LCD_REG = regno;
    LCD->LCD_RAM = data;
}

/**
 * @brief   LCD delay for optimization compensation (-O1 in MDK)
 * @param   i: delay count
 */
static void lcd_opt_delay(uint32_t i)
{
    while (i--);
}

/**
 * @brief   LCD read data
 * @retval  16-bit data read
 */
static uint16_t lcd_rd_data(void)
{
    volatile uint16_t ram;  /* prevent optimization */
    lcd_opt_delay(2);
    ram = LCD->LCD_RAM;
    return ram;
}

/**
 * @brief   Prepare to write GRAM
 */
void lcd_write_ram_prepare(void)
{
    LCD->LCD_REG = lcddev.wramcmd;
}

/**
 * @brief   Read pixel color at position
 * @param   x,y: coordinates
 * @retval  32-bit color value
 */
uint32_t lcd_read_point(uint16_t x, uint16_t y)
{
    uint16_t r = 0, g = 0, b = 0;

    if (x >= lcddev.width || y >= lcddev.height)
    {
        return 0;
    }

    lcd_set_cursor(x, y);

    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2E00);   /* 5510 read GRAM command */
    }
    else
    {
        lcd_wr_regno(0x2E);     /* 9341/5310/1963/7789/7796/9806 read GRAM */
    }

    r = lcd_rd_data();          /* dummy read */

    if (lcddev.id == 0x1963)
    {
        return r;               /* 1963 returns directly */
    }

    r = lcd_rd_data();          /* actual color value */

    if (lcddev.id == 0x7796)    /* 7796 reads one color value at a time */
    {
        return r;
    }

    /* 9341/5310/5510/7789/9806 need 2 reads */
    b = lcd_rd_data();
    g = r & 0xFF;               /* first read contains RG, R first, G second, 8 bits each */
    g <<= 8;

    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));
}

/**
 * @brief   Turn LCD display on
 */
void lcd_display_on(void)
{
    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2900);
    }
    else
    {
        lcd_wr_regno(0x29);
    }
}

/**
 * @brief   Turn LCD display off
 */
void lcd_display_off(void)
{
    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(0x2800);
    }
    else
    {
        lcd_wr_regno(0x28);
    }
}

/**
 * @brief   Set cursor position (for RGB screen)
 * @param   x,y: coordinates
 */
void lcd_set_cursor(uint16_t x, uint16_t y)
{
    if (lcddev.id == 0x1963)
    {
        if (lcddev.dir == 0)    /* landscape mode, x needs transform */
        {
            x = lcddev.width - 1 - x;
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(0);
            lcd_wr_data(0);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0xFF);
        }
        else                    /* portrait mode */
        {
            lcd_wr_regno(lcddev.setxcmd);
            lcd_wr_data(x >> 8);
            lcd_wr_data(x & 0xFF);
            lcd_wr_data((lcddev.width - 1) >> 8);
            lcd_wr_data((lcddev.width - 1) & 0xFF);
        }

        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0xFF);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_regno(lcddev.setxcmd + 1);
        lcd_wr_data(x & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_regno(lcddev.setycmd + 1);
        lcd_wr_data(y & 0xFF);
    }
    else    /* 9341/5310/7789/7796/9806 etc. */
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(x >> 8);
        lcd_wr_data(x & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(y >> 8);
        lcd_wr_data(y & 0xFF);
    }
}

/**
 * @brief   Set LCD auto scan direction (not for RGB screen)
 *   @note  Tested with 9341/5310/5510/1963/7789/7796/9806.
 *          Note: horizontal/vertical screen setting may affect this function
 *          (e.g. 9341). Default is L2R_U2D, others may cause display issues.
 * @param   dir: 0~7, 8 directions (see lcd.h)
 */
void lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t dirreg = 0;
    uint16_t temp;

    /* 1963 direction transform (1963 is special, other ICs not affected) */
    if ((lcddev.dir == 1 && lcddev.id != 0x1963) || (lcddev.dir == 0 && lcddev.id == 0x1963))
    {
        switch (dir)
        {
            case 0: dir = 6; break;
            case 1: dir = 7; break;
            case 2: dir = 4; break;
            case 3: dir = 5; break;
            case 4: dir = 1; break;
            case 5: dir = 0; break;
            case 6: dir = 3; break;
            case 7: dir = 2; break;
        }
    }

    /* Set scan direction bits (5,6,7) in register 0x36/0x3600 */
    switch (dir)
    {
        case L2R_U2D: /* left to right, top to bottom */
            regval |= (0 << 7) | (0 << 6) | (0 << 5); break;
        case L2R_D2U: /* left to right, bottom to top */
            regval |= (1 << 7) | (0 << 6) | (0 << 5); break;
        case R2L_U2D: /* right to left, top to bottom */
            regval |= (0 << 7) | (1 << 6) | (0 << 5); break;
        case R2L_D2U: /* right to left, bottom to top */
            regval |= (1 << 7) | (1 << 6) | (0 << 5); break;
        case U2D_L2R: /* top to bottom, left to right */
            regval |= (0 << 7) | (0 << 6) | (1 << 5); break;
        case U2D_R2L: /* top to bottom, right to left */
            regval |= (0 << 7) | (1 << 6) | (1 << 5); break;
        case D2U_L2R: /* bottom to top, left to right */
            regval |= (1 << 7) | (0 << 6) | (1 << 5); break;
        case D2U_R2L: /* bottom to top, right to left */
            regval |= (1 << 7) | (1 << 6) | (1 << 5); break;
    }

    dirreg = 0x36;

    if (lcddev.id == 0x5510)
    {
        dirreg = 0x3600;
    }

    /* 9341 & 7789 & 7796 need BGR bit */
    if (lcddev.id == 0x9341 || lcddev.id == 0x7789 || lcddev.id == 0x7796)
    {
        regval |= 0x08;
    }

    lcd_write_reg(dirreg, regval);

    if (lcddev.id != 0x1963)    /* 1963 handles coordinates separately */
    {
        if (regval & 0x20)
        {
            if (lcddev.width < lcddev.height)   /* swap X,Y */
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
        else
        {
            if (lcddev.width > lcddev.height)   /* swap X,Y */
            {
                temp = lcddev.width;
                lcddev.width = lcddev.height;
                lcddev.height = temp;
            }
        }
    }

    /* Set display window size */
    if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd); lcd_wr_data(0);
        lcd_wr_regno(lcddev.setxcmd + 1); lcd_wr_data(0);
        lcd_wr_regno(lcddev.setxcmd + 2); lcd_wr_data((lcddev.width - 1) >> 8);
        lcd_wr_regno(lcddev.setxcmd + 3); lcd_wr_data((lcddev.width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd); lcd_wr_data(0);
        lcd_wr_regno(lcddev.setycmd + 1); lcd_wr_data(0);
        lcd_wr_regno(lcddev.setycmd + 2); lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_regno(lcddev.setycmd + 3); lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
    else
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(0); lcd_wr_data(0);
        lcd_wr_data((lcddev.width - 1) >> 8);
        lcd_wr_data((lcddev.width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(0); lcd_wr_data(0);
        lcd_wr_data((lcddev.height - 1) >> 8);
        lcd_wr_data((lcddev.height - 1) & 0xFF);
    }
}

/**
 * @brief   Draw a point
 * @param   x,y: coordinates
 * @param   color: 32-bit color
 */
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color)
{
    lcd_set_cursor(x, y);
    lcd_write_ram_prepare();
    LCD->LCD_RAM = color;
}

/**
 * @brief   SSD1963 backlight control
 * @param   pwm: brightness level 0~100
 */
void lcd_ssd_backlight_set(uint8_t pwm)
{
    lcd_wr_regno(0xBE);
    lcd_wr_data(0x05);          /* PWM frequency */
    lcd_wr_data(pwm * 2.55);    /* PWM duty */
    lcd_wr_data(0x01);          /* C */
    lcd_wr_data(0xFF);          /* D */
    lcd_wr_data(0x00);          /* E */
    lcd_wr_data(0x00);          /* F */
}

/**
 * @brief   Set LCD display direction
 * @param   dir: 0=portrait, 1=landscape
 */
void lcd_display_dir(uint8_t dir)
{
    lcddev.dir = dir;

    if (dir == 0)       /* portrait */
    {
        lcddev.width = 240;
        lcddev.height = 320;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0x2C00;
            lcddev.setxcmd = 0x2A00;
            lcddev.setycmd = 0x2B00;
            lcddev.width = 480;
            lcddev.height = 800;
        }
        else if (lcddev.id == 0x1963)
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2B;
            lcddev.setycmd = 0x2A;
            lcddev.width = 480;
            lcddev.height = 800;
        }
        else   /* 9341/5310/7789/7796/9806 etc. */
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }

        if (lcddev.id == 0x5310 || lcddev.id == 0x7796)
        {
            lcddev.width = 320;
            lcddev.height = 480;
        }

        if (lcddev.id == 0X9806)
        {
            lcddev.width = 480;
            lcddev.height = 800;
        }
    }
    else        /* landscape */
    {
        lcddev.width = 320;
        lcddev.height = 240;

        if (lcddev.id == 0x5510)
        {
            lcddev.wramcmd = 0x2C00;
            lcddev.setxcmd = 0x2A00;
            lcddev.setycmd = 0x2B00;
            lcddev.width = 800;
            lcddev.height = 480;
        }
        else if (lcddev.id == 0x1963 || lcddev.id == 0x9806)
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
            lcddev.width = 800;
            lcddev.height = 480;
        }
        else   /* 9341/5310/7789/7796 etc. */
        {
            lcddev.wramcmd = 0x2C;
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }

        if (lcddev.id == 0x5310 || lcddev.id == 0x7796)
        {
            lcddev.width = 480;
            lcddev.height = 320;
        }
    }

    lcd_scan_dir(DFT_SCAN_DIR);
}

/**
 * @brief   Set drawing window
 * @param   sx,sy: top-left corner
 * @param   width,height: window dimensions (>0)
 */
void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint16_t twidth, theight;
    twidth = sx + width - 1;
    theight = sy + height - 1;

    if (lcddev.id == 0x1963 && lcddev.dir != 1)     /* 1963 special handling */
    {
        sx = lcddev.width - width - sx;
        height = sy + height - 1;
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8); lcd_wr_data(sx & 0xFF);
        lcd_wr_data((sx + width - 1) >> 8); lcd_wr_data((sx + width - 1) & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8); lcd_wr_data(sy & 0xFF);
        lcd_wr_data(height >> 8); lcd_wr_data(height & 0xFF);
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_wr_regno(lcddev.setxcmd); lcd_wr_data(sx >> 8);
        lcd_wr_regno(lcddev.setxcmd + 1); lcd_wr_data(sx & 0xFF);
        lcd_wr_regno(lcddev.setxcmd + 2); lcd_wr_data(twidth >> 8);
        lcd_wr_regno(lcddev.setxcmd + 3); lcd_wr_data(twidth & 0xFF);
        lcd_wr_regno(lcddev.setycmd); lcd_wr_data(sy >> 8);
        lcd_wr_regno(lcddev.setycmd + 1); lcd_wr_data(sy & 0xFF);
        lcd_wr_regno(lcddev.setycmd + 2); lcd_wr_data(theight >> 8);
        lcd_wr_regno(lcddev.setycmd + 3); lcd_wr_data(theight & 0xFF);
    }
    else    /* 9341/5310/7789/1963/7796/9806 etc. */
    {
        lcd_wr_regno(lcddev.setxcmd);
        lcd_wr_data(sx >> 8); lcd_wr_data(sx & 0xFF);
        lcd_wr_data(twidth >> 8); lcd_wr_data(twidth & 0xFF);
        lcd_wr_regno(lcddev.setycmd);
        lcd_wr_data(sy >> 8); lcd_wr_data(sy & 0xFF);
        lcd_wr_data(theight >> 8); lcd_wr_data(theight & 0xFF);
    }
}

/* HAL_SRAM_MspInit is provided by CubeMX in fsmc.c.
 * BSP no longer defines this function to avoid linker conflict. */

/**
 * @brief   Initialize LCD (detects LCD IC and configures FSMC timing)
 * @param   None
 * @retval  None
 */
void lcd_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    // FSMC_NORSRAM_TimingTypeDef fsmc_read_handle ;
     FSMC_NORSRAM_TimingTypeDef fsmc_write_handle ;

    LCD_BL_GPIO_CLK_ENABLE();   /* LCD_BL clock enable */
    gpio_init_struct.Pin = LCD_BL_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_BL_GPIO_PORT, &gpio_init_struct);
    /* CubeMX FSMC init already configures PD4/PD5/PD7/PD13 as AF12 (FSMC pins) */

    /* FSMC initialized by CubeMX-generated fsmc.c (MX_FSMC_Init), skip here */
    /* g_sram_handle.Instance = FSMC_NORSRAM_DEVICE;
    g_sram_handle.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

    g_sram_handle.Init.NSBank = FSMC_NORSRAM_BANK1;
    g_sram_handle.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    g_sram_handle.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    g_sram_handle.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    g_sram_handle.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    g_sram_handle.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    g_sram_handle.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    g_sram_handle.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    g_sram_handle.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
    g_sram_handle.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    g_sram_handle.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

    fsmc_read_handle.AddressSetupTime = 0x0F;
    fsmc_read_handle.AddressHoldTime = 0x00;
    fsmc_read_handle.DataSetupTime = 60;
    fsmc_read_handle.AccessMode = FSMC_ACCESS_MODE_A;

    fsmc_write_handle.AddressSetupTime = 9;
    fsmc_write_handle.AddressHoldTime = 0x00;
    fsmc_write_handle.DataSetupTime = 9;
    fsmc_write_handle.AccessMode = FSMC_ACCESS_MODE_A;

    HAL_SRAM_Init(&g_sram_handle, &fsmc_read_handle, &fsmc_write_handle);
    delay_ms(50); */

    /* Read ILI9341 ID */
    lcd_wr_regno(0xD3);
    lcddev.id = lcd_rd_data();  /* dummy read */
    lcddev.id = lcd_rd_data();  /* reads 0x00 */
    lcddev.id = lcd_rd_data();  /* reads 93 */
    lcddev.id <<= 8;
    lcddev.id |= lcd_rd_data(); /* reads 41 */

    if (lcddev.id != 0x9341)    /* not 9341, try ST7789 */
    {
        lcd_wr_regno(0x04);
        lcddev.id = lcd_rd_data();      /* dummy read */
        lcddev.id = lcd_rd_data();      /* reads 0x85 */
        lcddev.id = lcd_rd_data();      /* reads 0x85 */
        lcddev.id <<= 8;
        lcddev.id |= lcd_rd_data();     /* reads 0x52 */

        if (lcddev.id == 0x8552)        /* convert 8552 ID to 7789 */
        {
            lcddev.id = 0x7789;
        }

        if (lcddev.id != 0x7789)        /* not ST7789, try NT35310 */
        {
            lcd_wr_regno(0xD4);
            lcddev.id = lcd_rd_data();  /* dummy read */
            lcddev.id = lcd_rd_data();  /* reads 0x01 */
            lcddev.id = lcd_rd_data();  /* reads 0x53 */
            lcddev.id <<= 8;
            lcddev.id |= lcd_rd_data(); /* reads 0x10 */

            if (lcddev.id != 0x5310)    /* not NT35310, try ST7796 */
            {
                lcd_wr_regno(0XD3);
                lcddev.id = lcd_rd_data();  /* dummy read */
                lcddev.id = lcd_rd_data();  /* reads 0x00 */
                lcddev.id = lcd_rd_data();  /* reads 0x77 */
                lcddev.id <<= 8;
                lcddev.id |= lcd_rd_data(); /* reads 0x96 */

                if (lcddev.id != 0x7796)    /* not ST7796, try NT35510 */
                {
                    /* Command sequence from vendor */
                    lcd_write_reg(0xF000, 0x0055);
                    lcd_write_reg(0xF001, 0x00AA);
                    lcd_write_reg(0xF002, 0x0052);
                    lcd_write_reg(0xF003, 0x0008);
                    lcd_write_reg(0xF004, 0x0001);

                    lcd_wr_regno(0xC500);       /* read ID low byte */
                    lcddev.id = lcd_rd_data();  /* reads 0x80 */
                    lcddev.id <<= 8;

                    lcd_wr_regno(0xC501);       /* read ID high byte */
                    lcddev.id |= lcd_rd_data(); /* reads 0x00 */

                    delay_ms(5);    /* 0xC501 is reset command for 1963, wait for reset */

                    if (lcddev.id != 0x5510)    /* not NT5510, try ILI9806 */
                    {
                        lcd_wr_regno(0XD3);
                        lcddev.id = lcd_rd_data();  /* dummy read */
                        lcddev.id = lcd_rd_data();  /* reads 0x00 */
                        lcddev.id = lcd_rd_data();  /* reads 0x98 */
                        lcddev.id <<= 8;
                        lcddev.id |= lcd_rd_data(); /* reads 0x06 */

                        if (lcddev.id != 0x9806)    /* not ILI9806, try SSD1963 */
                        {
                            lcd_wr_regno(0xA1);
                            lcddev.id = lcd_rd_data();
                            lcddev.id = lcd_rd_data();  /* reads 0x57 */
                            lcddev.id <<= 8;
                            lcddev.id |= lcd_rd_data(); /* reads 0x61 */

                            if (lcddev.id == 0x5761) lcddev.id = 0x1963;
                        }
                    }
                }
            }
        }
    }

    printf("LCD ID:%x\r\n", lcddev.id); /* print LCD ID */

    /* Execute IC-specific register initialization */
    if (lcddev.id == 0x7789)
    {
        lcd_ex_st7789_reginit();
    }
    else if (lcddev.id == 0x9341)
    {
        lcd_ex_ili9341_reginit();
    }
    else if (lcddev.id == 0x5310)
    {
        lcd_ex_nt35310_reginit();
    }
    else if (lcddev.id == 0x7796)
    {
        lcd_ex_st7796_reginit();
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_ex_nt35510_reginit();
    }
    else if (lcddev.id == 0x9806)
    {
        lcd_ex_ili9806_reginit();
    }
    else if (lcddev.id == 0x1963)
    {
        lcd_ex_ssd1963_reginit();
        lcd_ssd_backlight_set(100);
    }

    /* Optimize write timing after init */
    if (lcddev.id == 0x7789 || lcddev.id == 0x9341 || lcddev.id == 0x1963)
    {
        fsmc_write_handle.AddressSetupTime = 3;
        fsmc_write_handle.DataSetupTime = 3;
        FSMC_NORSRAM_Extended_Timing_Init(g_sram_handle.Extended, &fsmc_write_handle,
                                           g_sram_handle.Init.NSBank, g_sram_handle.Init.ExtendedMode);
    }
    else if (lcddev.id == 0x5310 || lcddev.id == 0x7796 ||
             lcddev.id == 0x5510 || lcddev.id == 0x9806)
    {
        fsmc_write_handle.AddressSetupTime = 2;
        fsmc_write_handle.DataSetupTime = 2;
        FSMC_NORSRAM_Extended_Timing_Init(g_sram_handle.Extended, &fsmc_write_handle,
                                           g_sram_handle.Init.NSBank, g_sram_handle.Init.ExtendedMode);
    }

    lcd_display_dir(0); /* default: portrait */
    LCD_BL(1);          /* turn on backlight */
    lcd_clear(WHITE);
}

/**
 * @brief   Clear screen with color
 * @param   color: fill color
 */
void lcd_clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;

    totalpoint *= lcddev.height;
    lcd_set_cursor(0x00, 0x0000);
    lcd_write_ram_prepare();

    for (index = 0; index < totalpoint; index++)
    {
        LCD->LCD_RAM = color;
    }
}

/**
 * @brief   Fill rectangle with single color
 * @param   (sx,sy),(ex,ey): diagonal corners
 * @param   color: 32-bit color
 */
void lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color)
{
    uint16_t i, j;
    uint16_t xlen = 0;
    xlen = ex - sx + 1;

    for (i = sy; i <= ey; i++)
    {
        lcd_set_cursor(sx, i);
        lcd_write_ram_prepare();

        for (j = 0; j < xlen; j++)
        {
            LCD->LCD_RAM = color;
        }
    }
}

/**
 * @brief   Fill rectangle with color array
 * @param   (sx,sy),(ex,ey): diagonal corners
 * @param   color: color array pointer
 */
void lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
    // uint16_t height, width;
    // uint16_t i, j;

    // width = ex - sx + 1;
    // height = ey - sy + 1;

    // for (i = 0; i < height; i++)
    // {
    //     lcd_set_cursor(sx, sy + i);
    //     lcd_write_ram_prepare();

    //     for (j = 0; j < width; j++)
    //     {
    //         LCD->LCD_RAM = color[i * width + j];
    //     }
    // }
    uint32_t size;
    uint32_t i;

    size = (uint32_t)(ex - sx + 1) * (ey - sy + 1);

    lcd_set_window(sx, sy, ex - sx + 1, ey - sy + 1);
    lcd_write_ram_prepare();

    for (i = 0; i < size; i++)
    {
        LCD->LCD_RAM = color[i];
    }
}

/**
 * @brief   Draw line
 * @param   x1,y1: start point
 * @param   x2,y2: end point
 * @param   color: line color
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0) { incx = 1; }
    else if (delta_x == 0) { incx = 0; }
    else { incx = -1; delta_x = -delta_x; }

    if (delta_y > 0) { incy = 1; }
    else if (delta_y == 0) { incy = 0; }
    else { incy = -1; delta_y = -delta_y; }

    if (delta_x > delta_y) { distance = delta_x; }
    else { distance = delta_y; }

    for (t = 0; t <= distance + 1; t++)
    {
        lcd_draw_point(row, col, color);
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance) { xerr -= distance; row += incx; }
        if (yerr > distance) { yerr -= distance; col += incy; }
    }
}

/**
 * @brief   Draw horizontal line
 * @param   x,y: start point
 * @param   len: line length
 * @param   color: line color
 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    if ((len == 0) || (x > lcddev.width) || (y > lcddev.height))
    {
        return;
    }

    lcd_fill(x, y, x + len - 1, y, color);
}

/**
 * @brief   Draw rectangle outline
 * @param   x1,y1: top-left corner
 * @param   x2,y2: bottom-right corner
 * @param   color: outline color
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(x1, y1, x2, y1, color);
    lcd_draw_line(x1, y1, x1, y2, color);
    lcd_draw_line(x1, y2, x2, y2, color);
    lcd_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief   Draw circle outline
 * @param   x0,y0: center
 * @param   r: radius
 * @param   color: circle color
 */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a, b;
    int di;

    a = 0;
    b = r;
    di = 3 - (r << 1);

    while (a <= b)
    {
        lcd_draw_point(x0 + a, y0 - b, color);
        lcd_draw_point(x0 + b, y0 - a, color);
        lcd_draw_point(x0 + b, y0 + a, color);
        lcd_draw_point(x0 + a, y0 + b, color);
        lcd_draw_point(x0 - a, y0 + b, color);
        lcd_draw_point(x0 - b, y0 + a, color);
        lcd_draw_point(x0 - a, y0 - b, color);
        lcd_draw_point(x0 - b, y0 - a, color);
        a++;

        /* Bresenham circle algorithm */
        if (di < 0) { di += 4 * a + 6; }
        else { di += 10 + 4 * (a - b); b--; }
    }
}

/**
 * @brief   Draw filled circle
 * @param   x,y: center
 * @param   r: radius
 * @param   color: fill color
 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    uint32_t i;
    uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
    uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
    uint32_t xr = r;

    lcd_draw_hline(x - r, y, 2 * r, color);

    for (i = 1; i <= imax; i++)
    {
        if ((i * i + xr * xr) > sqmax)
        {
            if (xr > imax)
            {
                lcd_draw_hline(x - i + 1, y + xr, 2 * (i - 1), color);
                lcd_draw_hline(x - i + 1, y - xr, 2 * (i - 1), color);
            }

            xr--;
        }

        lcd_draw_hline(x - xr, y + i, 2 * xr, color);
        lcd_draw_hline(x - xr, y - i, 2 * xr, color);
    }
}

/**
 * @brief   Show a single character at position
 * @param   x,y: position
 * @param   chr: character " "~"~"
 * @param   size: font size 12/16/24/32
 * @param   mode: 1=superimpose, 0=non-superimpose
 * @param   color: character color
 */
void lcd_show_char(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;
    uint8_t csize = 0;
    uint8_t *pfont = 0;

    csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    chr = chr - ' ';

    switch (size)
    {
        case 12: pfont = (uint8_t *)asc2_1206[chr]; break;
        case 16: pfont = (uint8_t *)asc2_1608[chr]; break;
        case 24: pfont = (uint8_t *)asc2_2412[chr]; break;
        case 32: pfont = (uint8_t *)asc2_3216[chr]; break;
        default: return;
    }

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t];

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
            {
                lcd_draw_point(x, y, color);
            }
            else if (mode == 0)
            {
                lcd_draw_point(x, y, g_back_color);
            }

            temp <<= 1;
            y++;

            if (y >= lcddev.height) return;

            if ((y - y0) == size)
            {
                y = y0;
                x++;

                if (x >= lcddev.width) return;

                break;
            }
        }
    }
}

/**
 * @brief   Power function: m^n
 * @param   m: base
 * @param   n: exponent
 * @retval  m raised to n
 */
static uint32_t lcd_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--) { result *= m; }

    return result;
}

/**
 * @brief   Show a number
 * @param   x,y: start position
 * @param   num: value (0 ~ 2^32)
 * @param   len: digit count
 * @param   size: font size 12/16/24/32
 * @param   color: number color
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                lcd_show_char(x + (size / 2) * t, y, ' ', size, 0, color);
                continue;
            }
            else
            {
                enshow = 1;
            }
        }

        lcd_show_char(x + (size / 2) * t, y, temp + '0', size, 0, color);
    }
}

/**
 * @brief   Extended number display (with leading zero option)
 * @param   x,y: start position
 * @param   num: value (0 ~ 2^32)
 * @param   len: digit count
 * @param   size: font size 12/16/24/32
 * @param   mode: display mode
 *              [7]: 1=show leading zeros
 *              [6:1]: reserved
 *              [0]: 1=superimpose, 0=non-superimpose
 * @param   color: number color
 */
void lcd_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode, uint16_t color)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                if (mode & 0x80)
                {
                    lcd_show_char(x + (size / 2) * t, y, '0', size, mode & 0x01, color);
                }
                else
                {
                    lcd_show_char(x + (size / 2) * t, y, ' ', size, mode & 0x01, color);
                }

                continue;
            }
            else
            {
                enshow = 1;
            }
        }

        lcd_show_char(x + (size / 2) * t, y, temp + '0', size, mode & 0x01, color);
    }
}

/**
 * @brief   Show string
 * @param   x,y: start position
 * @param   width,height: bounding box
 * @param   size: font size 12/16/24/32
 * @param   p: string pointer
 * @param   color: text color
 */
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p, uint16_t color)
{
    uint8_t x0 = x;

    width += x;
    height += y;

    while ((*p <= '~') && (*p >= ' '))
    {
        if (x >= width) { x = x0; y += size; }
        if (y >= height) break;

        lcd_show_char(x, y, *p, size, 0, color);
        x += size / 2;
        p++;
    }
}
