#ifndef __LCD_H
#define __LCD_H

#include <stdbool.h>
#include "bsp_ili9341_lcd.h"

void LCD_DisplayInit(void);
void LCD_ShowBoot(void);
void LCD_ShowESPFail(void);
void LCD_UpdateSensor(float voltage, float threshold, bool led_on);
void LCD_ShowWiFiStatus(bool esp_ok);

#endif /* __LCD_H */
