#include "main.h"
#include "monitorrom.h"
#include "clockdemo.h"
#include "6502.c"

#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"
#include "hardware/adc.h"
#include "system1.c"
#include "hardware/watchdog.h"
#include "pico/multicore.h"


uint8_t breakKey = 0;
TP_DEV sTP_DEV;

void core1_main()
{
    // This routine waits t*10 milliseconds. t is read from memory[0xE2C].
    int t,i;
    while(true) {
        t = 100;
        i=0;
        printf(".");   // Why does the timer not decrement when this line is missing or printing nothing ???
        while (t > 0) {
            sleep_ms(10);
            gpio_put(0,i);
            i=i^1;
            t--;
        }
        memory[0xE2C] = 0;
    }
}

int main(void)
{
    int i;

    gpio_init(0);
    gpio_set_dir(0,1);

    set_sys_clock_khz(SYSCLK_MHZ * 1000, true);
    stdio_init_all();

	System_Init();

	adc_init();
	adc_set_temp_sensor_enabled(true);
	adc_select_input(4);

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
    memcpy(memory+0xFE00, monitor, 512);

    // Load the clock demo rom
    memcpy(memory+0x0200, clockdemo, 504);

    // Initilize the I/O, after a reset both ports of the 8154 are inputs
    // so at read they should return 0xFF
    memory[0x0E20] = 0xFF;
    memory[0xE021] = 0xFF;

    // Start timer function on core1
    multicore_launch_core1(core1_main);

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
