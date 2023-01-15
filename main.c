#include "main.h"
#include "monitorrom.h"
#include "6502.c"

#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"
#include "system1.c"

uint8_t breakKey = 0;
TP_DEV sTP_DEV;

int main(void)
{
    int i;

    set_sys_clock_khz(SYSCLK_MHZ * 1000, true);
    stdio_init_all();

	System_Init();

	LCD_SCAN_DIR  lcd_scan_dir = SCAN_DIR_DFT;
	LCD_Init(lcd_scan_dir,800);
	TP_Init(lcd_scan_dir);
	TP_GetAdFac();
    drawKeyboard();
	drawAcorn();

    // Draw display
    for(i=0; i<8; i++) {
        drawDigit(i, 0);
    }


    // Load the monitor rom
    memcpy(memory+0xFE00, monitor, 0x0200);

    // Initilize the I/O, after a reset both ports of the 8154 are inputs
    // so at read they should return 0xFF
    memory[0x0E20] = 0xFF;
    memory[0xE021] = 0xFF;

    reset6502();
    while(1) {
        step6502();
        if (breakKey == 1) {
            reset6502();
            breakKey = 0;
        }
    }
    return 0;
}
