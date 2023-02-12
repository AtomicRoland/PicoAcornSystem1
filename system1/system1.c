/*****************************************************************************
* | File      	:   lcd_2in8.c
* | Author      :   Waveshare team
* | Function    :   2.9inch e-paper V2 test demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-01-20
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "main.h"
#include "LCD_Driver.h"
#include "LCD_Touch.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"
#include "DEV_Config.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "/Users/roland/pico/pico-sdk/src/rp2_common/hardware_adc/include/hardware/adc.h"

extern uint8_t memory[0x10000];
extern uint8_t breakKey;

void drawKeyboard(void) {
    int x, y, offset;
    int i=0;

    char keylabels[] = "lmfedcrgba98<p7654>s3210X";

    // Clear the screen
    LCD_Clear(LCD_BACKGROUND);

    // Draw keyboard
    for (x=0; x<4; x++) {
        for (y=0; y<6; y++) {
            offset = y > 1 ? 20 : 0;
            GUI_DrawRectangle(x*30+120, y*50+4+offset, x*30+144, y*50+40+offset, YELLOW, DRAW_FULL, DOT_PIXEL_1X1);
            if (i == 12 || i == 18 ) {  // < and > are printed in other direction to make up and down instead of left and right
                GUI_DisChar(x*30+126, y*50+16+offset, keylabels[i++], &Font20, YELLOW, BLACK);
            } else {
                GUI_DisCharH(y*50+16+offset, x*30+124, keylabels[i++], &Font20, YELLOW, BLACK);
            }
        }
    }
    // Draw reset (X) button
    GUI_DrawRectangle(90, 4, 114, 40, YELLOW, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DisCharH(16, 94, keylabels[i++], &Font20, YELLOW, BLACK);
}

void drawLed(int x, int y, int status, char dir) {
    int color = status ? RED : GREY;
    if (dir == 'v') {
        GUI_DrawLine(x+1, y  , x+22, y,   color, LINE_SOLID, DOT_PIXEL_1X1);
        GUI_DrawLine(x  , y+1, x+24, y+1, color, LINE_SOLID, DOT_PIXEL_1X1);
        GUI_DrawLine(x+1, y+2, x+22, y+2, color, LINE_SOLID, DOT_PIXEL_1X1);
    }
    if (dir == 'h') {
        GUI_DrawLine(x  , y+1, x  , y+22, color, LINE_SOLID, DOT_PIXEL_1X1);
        GUI_DrawLine(x+1, y  , x+1, y+24, color, LINE_SOLID, DOT_PIXEL_1X1);
        GUI_DrawLine(x+2, y+1, x+2, y+22, color, LINE_SOLID, DOT_PIXEL_1X1);
    }
    if (dir == 'c') {
        GUI_DrawCircle(x, y, 2, color, DRAW_FULL , DOT_PIXEL_1X1);
    }
}

void drawDigit(int pos, int value) {
    static int ledstatus[] = {0, 0, 0, 0, 0, 0, 0, 0};
    int x, y;

    if (ledstatus[pos] != value) {
        y = (7-pos) * 40 + 8;
        x = 10;

        drawLed(x, y, value&1, 'h');            // digit a
        drawLed(x, y, value&2, 'v');            // digit b
        drawLed(x+24, y, value&4, 'v');         // digit c
        drawLed(x+48, y, value&8, 'h');         // digit d
        drawLed(x+24, y+24, value&16, 'v');     // digit e
        drawLed(x, y+24, value&32, 'v');        // digit f
        drawLed(x+24, y, value&64, 'h');        // digit g
        drawLed(x+48, y-4, value&128, 'c');     // digit h (dp)

        ledstatus[pos] = value;
    }
}

void drawAcorn(void) {
    char acorn[] = "Acorn";
    char system[] = "System1";
    uint8_t i,p;
    uint16_t acorndata[] = { 0x0080, 0x07E0, 0x0FF0, 0x1FF8, 0x1FF8, 0x3FFC, 0x3FFC, 0x3FFC,
                             0x7FFE, 0x7FFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,
                             0xFFFF, 0x3FFC, 0x0FF0, 0x0080, 0xC080, 0x3F80, 0x00FC, 0x0003 };

    for (i=0; i<5; i++) {
        GUI_DisCharH(240-i*20, 70, acorn[i], &Font24, WHITE, BLACK);
    }

    for (i=0; i<7; i++) {
        GUI_DisCharH(240-i*20, 90, system[i], &Font24, WHITE, BLACK);
    }

    for (i=0; i<24; i++) {
        for (p=0; p<16; p++) {
            if (acorndata[i] & 1) {
                GUI_DrawPoint(70+i*2, 275+p*2, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
            }
            acorndata[i] /= 2;
        }
    }

}

char keyDecode(int x, int y) {
    if (y>350 && y<390) {                       // reset
        if (x>220 && x<250) {
            breakKey = 1;
            return 'X';
        } else {
            breakKey = 0;
        }

        if (x>220 && x<250) {return 'X'; }      // reset
        if (x>170 && x<200) {return 'l'; }
        if (x>130 && x<160) {return 'r'; }
        if (x> 80 && x<110) {return 'u'; }
        if (x> 35 && x< 65) {return 'v'; }
    }

    if (y>295 && y<335) {
        if (x>170 && x<200) {return 'm'; }
        if (x>130 && x<160) {return 'g'; }
        if (x> 80 && x<110) {return 'p'; }
        if (x> 35 && x< 65) {return 's'; }
    }

    if (y>220 && y<260) {
        if (x>170 && x<200) {return 'f'; }
        if (x>130 && x<160) {return 'b'; }
        if (x> 80 && x<110) {return '7'; }
        if (x> 35 && x< 65) {return '3'; }
    }

    if (y>160 && y<200) {
        if (x>170 && x<200) {return 'e'; }
        if (x>130 && x<160) {return 'a'; }
        if (x> 80 && x<110) {return '6'; }
        if (x> 35 && x< 65) {return '2'; }
    }

    if (y>100 && y<140) {
        if (x>170 && x<200) {return 'd'; }
        if (x>130 && x<160) {return '9'; }
        if (x> 80 && x<110) {return '5'; }
        if (x> 35 && x< 65) {return '1'; }
    }

    if (y>45 && y<85) {
        if (x>170 && x<200) {return 'c'; }
        if (x>130 && x<160) {return '8'; }
        if (x> 80 && x<110) {return '4'; }
        if (x> 35 && x< 65) {return '0'; }
    }

    return '-';
}

void core1_iosubsystem(void) {
	int i,digit = 0;
	char key;
	uint8_t scanDrive, prevScanDrive = 0, keySense = 0;
	TP_DEV sTP_DEV;
   	
	System_Init();

	LCD_SCAN_DIR  lcd_scan_dir = SCAN_DIR_DFT;
	LCD_Init(lcd_scan_dir,800);
	TP_Init(lcd_scan_dir);
	TP_GetAdFac();
    drawKeyboard();

    // Draw display
    for(i=0; i<8; i++) {
        drawDigit(i, 0);
    }

	while(1){
	    scanDrive = memory[0XE20] & 0x07;

        // Display data: update the digit after the 6502 has written a new value to the
        // scan drive bits (bit 0..2 of 0XE20). Note that, unlike the real hardware, the
        // current digit stays "on". This emulator does not clear it. This avoids flickering
        // but might be incompatible with some programs.

	    if (scanDrive != prevScanDrive) {
	        prevScanDrive = scanDrive;
	        drawDigit(scanDrive, memory[0XE21]);
	    }

        sTP_DEV = TP_Scan(0);
        if(sTP_DEV.Xpoint < 5120 && sTP_DEV.Ypoint < 5120) {
            key = keyDecode(sTP_DEV.Xpoint/10, sTP_DEV.Ypoint/10);
//            printf("Touch x:%d,y:%d - Pressed: %c\n", sTP_DEV.Xpoint/10, sTP_DEV.Ypoint/10, key);
            if (key != '-') {
                // There is a key pressed, update keySense according to the scanDrive and pressed key
                keySense = 0x00;
                if (scanDrive == 0) {
                    if (key == '0') { keySense &= 0xDF; }
                    if (key == 'm') { keySense &= 0xEF; }
                    if (key == '8') { keySense &= 0xF7; }
                }

                if (scanDrive == 1) {
                    if (key == '1') { keySense &= 0xDF; }
                    if (key == 'g') { keySense &= 0xEF; }
                    if (key == '9') { keySense &= 0xF7; }
                }

                if (scanDrive == 2) {
                    if (key == '2') { keySense &= 0xDF; }
                    if (key == 'p') { keySense &= 0xEF; }
                    if (key == 'a') { keySense &= 0xF7; }
                }

                if (scanDrive == 3) {
                    if (key == '3') { keySense &= 0xDF; }
                    if (key == 's') { keySense &= 0xEF; }
                    if (key == 'b') { keySense &= 0xF7; }
                }

                if (scanDrive == 4) {
                    if (key == '4') { keySense &= 0xDF; }
                    if (key == 'l') { keySense &= 0xEF; }
                    if (key == 'c') { keySense &= 0xF7; }
                }

                if (scanDrive == 5) {
                    if (key == '5') { keySense &= 0xDF; }
                    if (key == 'r') { keySense &= 0xEF; }
                    if (key == 'd') { keySense &= 0xF7; }
                }

                if (scanDrive == 6) {
                    if (key == '6') { keySense &= 0xDF; }
                    if (key == 'u') { keySense &= 0xEF; }
                    if (key == 'e') { keySense &= 0xF7; }
                }

                if (scanDrive == 7) {
                    if (key == '7') { keySense &= 0xDF; }
                    if (key == 'v') { keySense &= 0xEF; }
                    if (key == 'f') { keySense &= 0xF7; }
                }
            } else {
                keySense = 56;      // Touchpad was pressed but not on a key
            }
        } else {
            keySense = 56;      // No key pressed
        }
        memory[0XE20] = 0xC0 | keySense | scanDrive;
//        sleep_us(100);
//        printf("Write E20: %02X sd=%02X ks=%02X\n", memory[0XE20], scanDrive, keySense);
    }
}


void readKey(void) {
	uint8_t scanDrive, keySense;
	char key;
	TP_DEV sTP_DEV;

    sTP_DEV = TP_Scan(0);
    scanDrive = memory[0xE20] & 0x07;
    if(sTP_DEV.Xpoint < 5120 && sTP_DEV.Ypoint < 5120) {
        key = keyDecode(sTP_DEV.Xpoint/10, sTP_DEV.Ypoint/10);
        if (key != '-') {
            // There is a key pressed, update keySense according to the scanDrive and pressed key
            keySense = 0x38;
            if (scanDrive == 0) {
                if (key == '0') { keySense &= 0xDF; }
                if (key == 'm') { keySense &= 0xEF; }
                if (key == '8') { keySense &= 0xF7; }
            }

            if (scanDrive == 1) {
                if (key == '1') { keySense &= 0xDF; }
                if (key == 'g') { keySense &= 0xEF; }
                if (key == '9') { keySense &= 0xF7; }
            }

            if (scanDrive == 2) {
                if (key == '2') { keySense &= 0xDF; }
                if (key == 'p') { keySense &= 0xEF; }
                if (key == 'a') { keySense &= 0xF7; }
            }

            if (scanDrive == 3) {
                if (key == '3') { keySense &= 0xDF; }
                if (key == 's') { keySense &= 0xEF; }
                if (key == 'b') { keySense &= 0xF7; }
            }

            if (scanDrive == 4) {
                if (key == '4') { keySense &= 0xDF; }
                if (key == 'l') { keySense &= 0xEF; }
                if (key == 'c') { keySense &= 0xF7; }
            }

            if (scanDrive == 5) {
                if (key == '5') { keySense &= 0xDF; }
                if (key == 'r') { keySense &= 0xEF; }
                if (key == 'd') { keySense &= 0xF7; }
            }

            if (scanDrive == 6) {
                if (key == '6') { keySense &= 0xDF; }
                if (key == 'u') { keySense &= 0xEF; }
                if (key == 'e') { keySense &= 0xF7; }
            }

            if (scanDrive == 7) {
                if (key == '7') { keySense &= 0xDF; }
                if (key == 'v') { keySense &= 0xEF; }
                if (key == 'f') { keySense &= 0xF7; }
            }
        } else {
            keySense = 0x38;      // Touchpad was pressed but not on a key
        }
    } else {
        keySense = 0x38;      // No key pressed
    }
    memory[0XE20] = 0xC0 | keySense | scanDrive;
}

void readTemp(void) {
    uint16_t raw = adc_read();
    char tempStr[3];
    const float conversion_factor = 3.3f / (1<<12);
    float result = raw * conversion_factor;
    float temp = 24.5 - (result - 0.706)/0.001721;
    sprintf(tempStr, "%03d", (int) (temp*10.0));
    printf("Temp = %f C   tempStr = %s\n", temp, tempStr);
    memory[0xE28] = 'T';
    memory[0xE29] = tempStr[0] - 0x30;
    memory[0xE2A] = tempStr[1] - 0x30;
    memory[0xE2B] = tempStr[2] - 0x30;
}