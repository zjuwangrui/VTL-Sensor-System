#include "lcd.h"
#include <stdio.h>
#include <string.h>

/* ===== 布局常量（横屏 320×240，Font16x24 / Font8x16） ===== */
#define ROW_TITLE_Y   4
#define ROW_V_Y       36    /* Voltage 行 */
#define ROW_THR_Y     56    /* Threshold 行 */
#define ROW_STATUS_Y  84    /* 状态色块 */
#define ROW_DESC_Y    128   /* 状态描述 */
#define ROW_WIFI_Y    160   /* WiFi 状态 */

#define COL_LABEL     4
#define COL_VALUE     160

static void draw_title(void)
{
    LCD_SetColors(WHITE, BLUE);
    LCD_SetFont(&Font8x16);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, 28);
    ILI9341_DispString_EN(20, ROW_TITLE_Y + 6, "VTL Sensor System");
}

static void draw_static_labels(void)
{
    LCD_SetColors(BLACK, WHITE);
    LCD_SetFont(&Font8x16);
    ILI9341_DispString_EN(COL_LABEL, ROW_V_Y,   "Voltage :");
    ILI9341_DispString_EN(COL_LABEL, ROW_THR_Y, "Threshold:");
}

void LCD_DisplayInit(void)
{
    ILI9341_Init();
    ILI9341_GramScan(6);                          /* 横屏 320×240 */
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    ILI9341_BackLed_Control(ENABLE);
    draw_title();
    draw_static_labels();
}

void LCD_ShowBoot(void)
{
    LCD_SetColors(YELLOW, WHITE);
    LCD_SetFont(&Font8x16);
    ILI9341_Clear(COL_LABEL, ROW_WIFI_Y, LCD_X_LENGTH - COL_LABEL, 20);
    ILI9341_DispString_EN(COL_LABEL, ROW_WIFI_Y, "WiFi: Initializing...");
}

void LCD_ShowESPFail(void)
{
    /* 红色全屏提示 */
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    LCD_SetColors(WHITE, RED);
    LCD_SetFont(&Font16x24);
    ILI9341_DispString_EN(30, 80,  "ESP8266 INIT");
    ILI9341_DispString_EN(60, 110, "  FAILED  ");
    LCD_SetFont(&Font8x16);
    ILI9341_DispString_EN(20, 150, "Check wiring & power");
}

void LCD_UpdateSensor(float voltage, float threshold, bool led_on)
{
    char buf[24];

    /* --- 电压数值 --- */
    LCD_SetFont(&Font8x16);
    LCD_SetColors(BLUE, WHITE);
    ILI9341_Clear(COL_VALUE, ROW_V_Y, LCD_X_LENGTH - COL_VALUE, 18);
    snprintf(buf, sizeof(buf), "%.3f V", voltage);
    ILI9341_DispString_EN(COL_VALUE, ROW_V_Y, buf);

    /* --- 阈值数值（仅手机下发新值时变化，变了才重绘） --- */
    static float last_thr = -1.0f;
    if (threshold != last_thr) {
        last_thr = threshold;
        LCD_SetColors(BLACK, WHITE);
        ILI9341_Clear(COL_VALUE, ROW_THR_Y, LCD_X_LENGTH - COL_VALUE, 18);
        snprintf(buf, sizeof(buf), "%.3f V", threshold);
        ILI9341_DispString_EN(COL_VALUE, ROW_THR_Y, buf);
    }

    /* --- 状态色块 + 说明（仅状态变化时重绘） --- */
    static int8_t last_led = -1;  /* -1 = 未初始化，强制首次绘制 */
    if ((int8_t)led_on != last_led) {
        last_led = (int8_t)led_on;
        ILI9341_Clear(0, ROW_STATUS_Y, LCD_X_LENGTH, 100);
        if (led_on) {
            /* 超过阈值：红色色块 */
            LCD_SetColors(WHITE, RED);
            LCD_SetFont(&Font16x24);
            ILI9341_DrawRectangle(10, ROW_STATUS_Y, 300, 36, 1);
            LCD_SetColors(WHITE, RED);
            ILI9341_DispString_EN(70, ROW_STATUS_Y + 6, "! ALERT !");
            LCD_SetColors(RED, WHITE);
            LCD_SetFont(&Font8x16);
            ILI9341_DispString_EN(COL_LABEL, ROW_DESC_Y,
                                  "Red LED ON: voltage > threshold");
        } else {
            /* 正常：绿色色块 */
            LCD_SetColors(WHITE, GREEN);
            LCD_SetFont(&Font16x24);
            ILI9341_DrawRectangle(10, ROW_STATUS_Y, 300, 36, 1);
            LCD_SetColors(WHITE, GREEN);
            ILI9341_DispString_EN(70, ROW_STATUS_Y + 6, " NORMAL ");
            LCD_SetColors(GREEN, WHITE);
            LCD_SetFont(&Font8x16);
            ILI9341_DispString_EN(COL_LABEL, ROW_DESC_Y,
                                  "Green LED ON: voltage < threshold");
        }
    }
}

void LCD_ShowWiFiStatus(bool esp_ok)
{
    ILI9341_Clear(COL_LABEL, ROW_WIFI_Y, LCD_X_LENGTH - COL_LABEL, 20);
    LCD_SetFont(&Font8x16);
    if (esp_ok) {
        LCD_SetColors(GREEN, WHITE);
        ILI9341_DispString_EN(COL_LABEL, ROW_WIFI_Y, "WiFi: OK  Green=init success");
    } else {
        LCD_SetColors(RED, WHITE);
        ILI9341_DispString_EN(COL_LABEL, ROW_WIFI_Y, "WiFi: FAIL");
    }
}
