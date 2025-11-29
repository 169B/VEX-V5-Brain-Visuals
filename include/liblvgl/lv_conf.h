/**
 * @file lv_conf.h
 * @brief LVGL Configuration for VEX V5 Brain Host Mode
 * 
 * This configuration is based on LVGL 8.3 and targets the VEX V5 Brain display
 * specifications: 480x272 pixels, 16-bit color (RGB565)
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888) */
#define LV_COLOR_DEPTH 16

/* Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI) */
#define LV_COLOR_16_SWAP 0

/* Enable more complex drawing routines */
#define LV_DRAW_COMPLEX 1

/* 1: Use custom malloc/free, 0: use standard malloc/free */
#define LV_MEM_CUSTOM 0

/* Size of the memory available for `lv_mem_alloc()` in bytes (>= 2kB) */
#define LV_MEM_SIZE (48U * 1024U)

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period in milliseconds */
#define LV_DISP_DEF_REFR_PERIOD 16

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD 30

/* Use a custom tick source that tells the elapsed time in milliseconds */
#define LV_TICK_CUSTOM 0

/*====================
 * DISPLAY SETTINGS
 *====================*/

/* VEX V5 Brain display resolution */
#define LV_HOR_RES_MAX 480
#define LV_VER_RES_MAX 272

/*====================
 * FEATURE CONFIGURATION
 *====================*/

/* Enable logging */
#define LV_USE_LOG 1

#if LV_USE_LOG
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
    #define LV_LOG_PRINTF 1
#endif

/* Asserts */
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

/*====================
 * FONT USAGE
 *====================*/

#define LV_FONT_MONTSERRAT_8 0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*====================
 * WIDGET USAGE
 *====================*/

#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 0
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TABLE 1
#define LV_USE_TEXTAREA 1

/* Extra widgets */
#define LV_USE_ANIMIMG 0
#define LV_USE_CALENDAR 0
#define LV_USE_CHART 1
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN 0
#define LV_USE_KEYBOARD 0
#define LV_USE_LED 1
#define LV_USE_LIST 1
#define LV_USE_MENU 0
#define LV_USE_METER 0
#define LV_USE_MSGBOX 1
#define LV_USE_SPAN 0
#define LV_USE_SPINBOX 0
#define LV_USE_SPINNER 1
#define LV_USE_TABVIEW 1
#define LV_USE_TILEVIEW 0
#define LV_USE_WIN 0

/*====================
 * THEME USAGE
 *====================*/

#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0

#endif /* LV_CONF_H */
