#include "lcd.h"
#include <stdio.h>
#include <string.h>

/* ===== 布局（横屏 320×240，Font8x16，每行 16px，间隔 18px） ===== */
#define ROW_TITLE_Y    2    /* 标题栏（高 24px） */
#define ROW_V_Y       26    /* 电压 */
#define ROW_V_THR_Y   44    /* 电压阈值 */
#define ROW_L_Y       62    /* 光照 */
#define ROW_L_THR_Y   80    /* 光照阈值 */
#define ROW_T_Y       98    /* 温度 */
#define ROW_T_THR_Y  116    /* 温度阈值 */
#define ROW_STATUS_Y 154    /* 状态色块（高 28px） */
#define ROW_DESC_Y   186    /* 状态说明 */
#define ROW_WIFI_Y   204    /* WiFi 状态 */

#define COL_LABEL      4
#define COL_VALUE    140

static void draw_title(void)
{
    LCD_SetColors(WHITE, BLUE);
    LCD_SetFont(&Font8x16);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, 24);
    ILI9341_DispString_EN(20, ROW_TITLE_Y + 4, "VTL Sensor System");
}

static void draw_static_labels(void)
{
    LCD_SetColors(BLACK, WHITE);
    LCD_SetFont(&Font8x16);
    ILI9341_DispString_EN(COL_LABEL, ROW_V_Y,     "Voltage  :");
    ILI9341_DispString_EN(COL_LABEL, ROW_V_THR_Y, "V-Thresh :");
    ILI9341_DispString_EN(COL_LABEL, ROW_L_Y,     "Light    :");
    ILI9341_DispString_EN(COL_LABEL, ROW_L_THR_Y, "L-Thresh :");
    ILI9341_DispString_EN(COL_LABEL, ROW_T_Y,     "Temp     :");
    ILI9341_DispString_EN(COL_LABEL, ROW_T_THR_Y, "T-Thresh :");
}

void LCD_DisplayInit(void)
{
    ILI9341_Init();
    ILI9341_GramScan(6);
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    ILI9341_BackLed_Control(ENABLE);
    draw_title();
    draw_static_labels();
}

void LCD_ShowBoot(void)
{
    LCD_SetColors(YELLOW, WHITE);
    LCD_SetFont(&Font8x16);
    ILI9341_Clear(COL_LABEL, ROW_WIFI_Y, LCD_X_LENGTH - COL_LABEL, 18);
    ILI9341_DispString_EN(COL_LABEL, ROW_WIFI_Y, "WiFi: Initializing...");
}

void LCD_ShowESPFail(void)
{
    ILI9341_Clear(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
    LCD_SetColors(WHITE, RED);
    LCD_SetFont(&Font16x24);
    ILI9341_DispString_EN(30,  80, "ESP8266 INIT");
    ILI9341_DispString_EN(60, 110, "  FAILED  ");
    LCD_SetFont(&Font8x16);
    ILI9341_DispString_EN(20, 150, "Check wiring & power");
}

void LCD_UpdateSensor(float voltage, float v_thr,
                      uint32_t light, uint32_t l_thr,
                      uint8_t temp, uint8_t temp_thr,
                      bool led_on)
{
    char buf[24];
    LCD_SetFont(&Font8x16);

    /* --- 电压（实时更新） --- */
    LCD_SetColors(BLUE, WHITE);
    ILI9341_Clear(COL_VALUE, ROW_V_Y, LCD_X_LENGTH - COL_VALUE, 16);
    snprintf(buf, sizeof(buf), "%.3f V", voltage);
    ILI9341_DispString_EN(COL_VALUE, ROW_V_Y, buf);

    /* --- 电压阈值（值变才重绘） --- */
    static float last_v_thr = -1.0f;
    if (v_thr != last_v_thr) {
        last_v_thr = v_thr;
        LCD_SetColors(BLACK, WHITE);
        ILI9341_Clear(COL_VALUE, ROW_V_THR_Y, LCD_X_LENGTH - COL_VALUE, 16);
        snprintf(buf, sizeof(buf), "%.3f V", v_thr);
        ILI9341_DispString_EN(COL_VALUE, ROW_V_THR_Y, buf);
    }

    /* --- 光照（实时更新） --- */
    LCD_SetColors(BLUE, WHITE);
    ILI9341_Clear(COL_VALUE, ROW_L_Y, LCD_X_LENGTH - COL_VALUE, 16);
    snprintf(buf, sizeof(buf), "%4lu", (unsigned long)light);
    ILI9341_DispString_EN(COL_VALUE, ROW_L_Y, buf);

    /* --- 光照阈值（值变才重绘） --- */
    static uint32_t last_l_thr = 0xFFFFFFFFU;
    if (l_thr != last_l_thr) {
        last_l_thr = l_thr;
        LCD_SetColors(BLACK, WHITE);
        ILI9341_Clear(COL_VALUE, ROW_L_THR_Y, LCD_X_LENGTH - COL_VALUE, 16);
        snprintf(buf, sizeof(buf), "%4lu", (unsigned long)l_thr);
        ILI9341_DispString_EN(COL_VALUE, ROW_L_THR_Y, buf);
    }

    /* --- 温度（实时更新） --- */
    LCD_SetColors(BLUE, WHITE);
    ILI9341_Clear(COL_VALUE, ROW_T_Y, LCD_X_LENGTH - COL_VALUE, 16);
    snprintf(buf, sizeof(buf), "%d C", (int)temp);
    ILI9341_DispString_EN(COL_VALUE, ROW_T_Y, buf);

    /* --- 温度阈值（值变才重绘） --- */
    static uint8_t last_t_thr = 0xFFU;
    if (temp_thr != last_t_thr) {
        last_t_thr = temp_thr;
        LCD_SetColors(BLACK, WHITE);
        ILI9341_Clear(COL_VALUE, ROW_T_THR_Y, LCD_X_LENGTH - COL_VALUE, 16);
        snprintf(buf, sizeof(buf), "%d C", (int)temp_thr);
        ILI9341_DispString_EN(COL_VALUE, ROW_T_THR_Y, buf);
    }

    /* --- 状态色块（仅状态翻转时重绘） --- */
    static int8_t last_led = -1;
    if ((int8_t)led_on != last_led) {
        last_led = (int8_t)led_on;
        ILI9341_Clear(0, ROW_STATUS_Y, LCD_X_LENGTH, 50);
        if (led_on) {
            LCD_SetColors(WHITE, RED);
            LCD_SetFont(&Font16x24);
            ILI9341_DrawRectangle(10, ROW_STATUS_Y, 300, 28, 1);
            ILI9341_DispString_EN(70, ROW_STATUS_Y + 2, "! ALERT !");
            LCD_SetColors(RED, WHITE);
            LCD_SetFont(&Font8x16);
            ILI9341_DispString_EN(COL_LABEL, ROW_DESC_Y,
                                  "Red: V or Light or Temp over thr");
        } else {
            LCD_SetColors(WHITE, GREEN);
            LCD_SetFont(&Font16x24);
            ILI9341_DrawRectangle(10, ROW_STATUS_Y, 300, 28, 1);
            ILI9341_DispString_EN(70, ROW_STATUS_Y + 2, " NORMAL ");
            LCD_SetColors(GREEN, WHITE);
            LCD_SetFont(&Font8x16);
            ILI9341_DispString_EN(COL_LABEL, ROW_DESC_Y,
                                  "Grn: V,Light,Temp all normal    ");
        }
    }
}

void LCD_ShowWiFiStatus(bool esp_ok)
{
    ILI9341_Clear(COL_LABEL, ROW_WIFI_Y, LCD_X_LENGTH - COL_LABEL, 18);
    LCD_SetFont(&Font8x16);
    if (esp_ok) {
        LCD_SetColors(GREEN, WHITE);
        ILI9341_DispString_EN(COL_LABEL, ROW_WIFI_Y, "WiFi:OK  Green LED=init OK");
    } else {
        LCD_SetColors(RED, WHITE);
        ILI9341_DispString_EN(COL_LABEL, ROW_WIFI_Y, "WiFi: FAIL");
    }
}
