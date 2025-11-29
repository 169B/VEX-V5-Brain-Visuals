/**
 * @file lvgl.h
 * @brief LVGL 8.3 Type Definitions and Function Stubs for Host Mode
 * 
 * This header provides the necessary LVGL types and function declarations
 * to enable compilation of PROS code in host mode. The actual implementation
 * stubs are designed to work with the host display driver.
 */

#ifndef LVGL_H
#define LVGL_H

#include "lv_conf.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*====================
 * TYPE DEFINITIONS
 *====================*/

/* Coordinate type */
typedef int16_t lv_coord_t;

/* Color types */
typedef union {
    struct {
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
    } ch;
    uint16_t full;
} lv_color_t;

typedef uint8_t lv_opa_t;

#define LV_OPA_TRANSP 0
#define LV_OPA_0 0
#define LV_OPA_10 25
#define LV_OPA_20 51
#define LV_OPA_30 76
#define LV_OPA_40 102
#define LV_OPA_50 127
#define LV_OPA_60 153
#define LV_OPA_70 178
#define LV_OPA_80 204
#define LV_OPA_90 229
#define LV_OPA_100 255
#define LV_OPA_COVER 255

/* Make color from RGB */
static inline lv_color_t lv_color_make(uint8_t r, uint8_t g, uint8_t b) {
    lv_color_t color;
    color.ch.red = r >> 3;
    color.ch.green = g >> 2;
    color.ch.blue = b >> 3;
    return color;
}

#define LV_COLOR_MAKE(r, g, b) lv_color_make(r, g, b)

/* Predefined colors */
#define lv_color_white() lv_color_make(255, 255, 255)
#define lv_color_black() lv_color_make(0, 0, 0)
#define lv_color_hex(c) lv_color_make((uint8_t)((c >> 16) & 0xFF), (uint8_t)((c >> 8) & 0xFF), (uint8_t)(c & 0xFF))

/* Area type */
typedef struct {
    lv_coord_t x1;
    lv_coord_t y1;
    lv_coord_t x2;
    lv_coord_t y2;
} lv_area_t;

/* Point type */
typedef struct {
    lv_coord_t x;
    lv_coord_t y;
} lv_point_t;

/*====================
 * OBJECT TYPES
 *====================*/

/* Forward declarations */
struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

struct _lv_disp_t;
typedef struct _lv_disp_t lv_disp_t;

struct _lv_indev_t;
typedef struct _lv_indev_t lv_indev_t;

struct _lv_group_t;
typedef struct _lv_group_t lv_group_t;

struct _lv_event_t;
typedef struct _lv_event_t lv_event_t;

/* Style type - simplified */
typedef struct {
    void* values;
    uint16_t prop_cnt;
} lv_style_t;

/* Object structure - simplified */
struct _lv_obj_t {
    struct _lv_obj_t* parent;
    lv_area_t coords;
    void* user_data;
    lv_style_t* styles;
    uint32_t state;
    uint32_t flags;
};

/* Font type - simplified */
typedef struct {
    uint8_t line_height;
    uint8_t base_line;
} lv_font_t;

/* Default fonts */
extern const lv_font_t lv_font_montserrat_12;
extern const lv_font_t lv_font_montserrat_14;
extern const lv_font_t lv_font_montserrat_16;

/*====================
 * DISPLAY DRIVER
 *====================*/

/* Display draw buffer */
typedef struct {
    void* buf1;
    void* buf2;
    void* buf_act;
    uint32_t size;
    lv_area_t area;
    volatile int flushing;
    volatile int flushing_last;
} lv_disp_draw_buf_t;

/* Display driver structure */
struct _lv_disp_drv_t;
typedef struct _lv_disp_drv_t lv_disp_drv_t;

typedef void (*lv_disp_drv_flush_cb_t)(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p);

struct _lv_disp_drv_t {
    lv_coord_t hor_res;
    lv_coord_t ver_res;
    lv_disp_draw_buf_t* draw_buf;
    uint32_t direct_mode : 1;
    uint32_t full_refresh : 1;
    uint32_t sw_rotate : 1;
    uint32_t antialiasing : 1;
    uint32_t rotated : 2;
    uint32_t screen_transp : 1;
    uint32_t dpi : 10;
    lv_disp_drv_flush_cb_t flush_cb;
    void* user_data;
};

/* Display structure */
struct _lv_disp_t {
    lv_disp_drv_t* driver;
    lv_obj_t* act_scr;
    lv_obj_t* prev_scr;
    lv_obj_t* scr_to_load;
    uint32_t last_activity_time;
};

/* Display driver functions */
void lv_disp_draw_buf_init(lv_disp_draw_buf_t* draw_buf, void* buf1, void* buf2, uint32_t size_in_px_cnt);
void lv_disp_drv_init(lv_disp_drv_t* driver);
lv_disp_t* lv_disp_drv_register(lv_disp_drv_t* driver);
void lv_disp_flush_ready(lv_disp_drv_t* disp_drv);

/*====================
 * INPUT DEVICE DRIVER
 *====================*/

/* Input device state */
typedef enum {
    LV_INDEV_STATE_REL = 0,
    LV_INDEV_STATE_PR
} lv_indev_state_t;

/* Input device type */
typedef enum {
    LV_INDEV_TYPE_NONE = 0,
    LV_INDEV_TYPE_POINTER,
    LV_INDEV_TYPE_KEYPAD,
    LV_INDEV_TYPE_BUTTON,
    LV_INDEV_TYPE_ENCODER
} lv_indev_type_t;

/* Input device data */
typedef struct {
    lv_point_t point;
    uint32_t key;
    uint32_t btn_id;
    int16_t enc_diff;
    lv_indev_state_t state;
    bool continue_reading;
} lv_indev_data_t;

/* Input driver structure */
struct _lv_indev_drv_t;
typedef struct _lv_indev_drv_t lv_indev_drv_t;

typedef void (*lv_indev_read_cb_t)(lv_indev_drv_t* drv, lv_indev_data_t* data);

struct _lv_indev_drv_t {
    lv_indev_type_t type;
    lv_indev_read_cb_t read_cb;
    void* user_data;
    lv_disp_t* disp;
    uint8_t read_timer_period;
};

/* Input device structure */
struct _lv_indev_t {
    lv_indev_drv_t* driver;
    lv_obj_t* cursor;
    lv_group_t* group;
    lv_point_t point;
    lv_indev_state_t state;
};

/* Input device functions */
void lv_indev_drv_init(lv_indev_drv_t* driver);
lv_indev_t* lv_indev_drv_register(lv_indev_drv_t* driver);

/*====================
 * EVENT SYSTEM
 *====================*/

/* Event codes */
typedef enum {
    LV_EVENT_ALL = 0,
    LV_EVENT_PRESSED,
    LV_EVENT_PRESSING,
    LV_EVENT_PRESS_LOST,
    LV_EVENT_SHORT_CLICKED,
    LV_EVENT_LONG_PRESSED,
    LV_EVENT_LONG_PRESSED_REPEAT,
    LV_EVENT_CLICKED,
    LV_EVENT_RELEASED,
    LV_EVENT_SCROLL_BEGIN,
    LV_EVENT_SCROLL_END,
    LV_EVENT_SCROLL,
    LV_EVENT_GESTURE,
    LV_EVENT_KEY,
    LV_EVENT_FOCUSED,
    LV_EVENT_DEFOCUSED,
    LV_EVENT_LEAVE,
    LV_EVENT_HIT_TEST,
    LV_EVENT_VALUE_CHANGED,
    LV_EVENT_INSERT,
    LV_EVENT_REFRESH,
    LV_EVENT_READY,
    LV_EVENT_CANCEL,
    LV_EVENT_DELETE,
    LV_EVENT_CHILD_CHANGED,
    LV_EVENT_CHILD_CREATED,
    LV_EVENT_CHILD_DELETED,
    LV_EVENT_SCREEN_UNLOAD_START,
    LV_EVENT_SCREEN_LOAD_START,
    LV_EVENT_SCREEN_LOADED,
    LV_EVENT_SCREEN_UNLOADED,
    LV_EVENT_SIZE_CHANGED,
    LV_EVENT_STYLE_CHANGED,
    LV_EVENT_LAYOUT_CHANGED,
    LV_EVENT_GET_SELF_SIZE,
    _LV_EVENT_LAST
} lv_event_code_t;

typedef void (*lv_event_cb_t)(lv_event_t* e);

struct _lv_event_t {
    lv_obj_t* target;
    lv_obj_t* current_target;
    lv_event_code_t code;
    void* user_data;
    void* param;
    bool deleted;
    bool stop_bubbling;
    bool stop_processing;
};

/* Event functions */
lv_event_code_t lv_event_get_code(lv_event_t* e);
lv_obj_t* lv_event_get_target(lv_event_t* e);
lv_obj_t* lv_event_get_current_target(lv_event_t* e);
void* lv_event_get_user_data(lv_event_t* e);
void* lv_event_get_param(lv_event_t* e);

/*====================
 * CORE FUNCTIONS
 *====================*/

/* Init and tick */
void lv_init(void);
void lv_deinit(void);
void lv_tick_inc(uint32_t tick_period);
uint32_t lv_tick_get(void);
void lv_timer_handler(void);

/* Screen functions */
lv_obj_t* lv_scr_act(void);
void lv_scr_load(lv_obj_t* scr);
void lv_scr_load_anim(lv_obj_t* scr, int anim_type, uint32_t time, uint32_t delay, bool auto_del);

/*====================
 * OBJECT FUNCTIONS
 *====================*/

/* Flags */
typedef enum {
    LV_OBJ_FLAG_HIDDEN = (1 << 0),
    LV_OBJ_FLAG_CLICKABLE = (1 << 1),
    LV_OBJ_FLAG_CLICK_FOCUSABLE = (1 << 2),
    LV_OBJ_FLAG_CHECKABLE = (1 << 3),
    LV_OBJ_FLAG_SCROLLABLE = (1 << 4),
    LV_OBJ_FLAG_SCROLL_ELASTIC = (1 << 5),
    LV_OBJ_FLAG_SCROLL_MOMENTUM = (1 << 6),
    LV_OBJ_FLAG_SCROLL_ONE = (1 << 7),
    LV_OBJ_FLAG_SCROLL_CHAIN = (1 << 8),
    LV_OBJ_FLAG_SCROLL_ON_FOCUS = (1 << 9),
    LV_OBJ_FLAG_SNAPPABLE = (1 << 11),
    LV_OBJ_FLAG_PRESS_LOCK = (1 << 12),
    LV_OBJ_FLAG_EVENT_BUBBLE = (1 << 13),
    LV_OBJ_FLAG_GESTURE_BUBBLE = (1 << 14),
    LV_OBJ_FLAG_ADV_HITTEST = (1 << 15),
    LV_OBJ_FLAG_IGNORE_LAYOUT = (1 << 16),
    LV_OBJ_FLAG_FLOATING = (1 << 17),
    LV_OBJ_FLAG_OVERFLOW_VISIBLE = (1 << 18),
    LV_OBJ_FLAG_LAYOUT_1 = (1 << 23),
    LV_OBJ_FLAG_LAYOUT_2 = (1 << 24),
    LV_OBJ_FLAG_WIDGET_1 = (1 << 25),
    LV_OBJ_FLAG_WIDGET_2 = (1 << 26),
    LV_OBJ_FLAG_USER_1 = (1 << 27),
    LV_OBJ_FLAG_USER_2 = (1 << 28),
    LV_OBJ_FLAG_USER_3 = (1 << 29),
    LV_OBJ_FLAG_USER_4 = (1 << 30),
} lv_obj_flag_t;

/* States */
typedef enum {
    LV_STATE_DEFAULT = 0x0000,
    LV_STATE_CHECKED = 0x0001,
    LV_STATE_FOCUSED = 0x0002,
    LV_STATE_FOCUS_KEY = 0x0004,
    LV_STATE_EDITED = 0x0008,
    LV_STATE_HOVERED = 0x0010,
    LV_STATE_PRESSED = 0x0020,
    LV_STATE_SCROLLED = 0x0040,
    LV_STATE_DISABLED = 0x0080,
    LV_STATE_USER_1 = 0x1000,
    LV_STATE_USER_2 = 0x2000,
    LV_STATE_USER_3 = 0x4000,
    LV_STATE_USER_4 = 0x8000,
    LV_STATE_ANY = 0xFFFF,
} lv_state_t;

/* Part identifiers */
typedef enum {
    LV_PART_MAIN = 0x000000,
    LV_PART_SCROLLBAR = 0x010000,
    LV_PART_INDICATOR = 0x020000,
    LV_PART_KNOB = 0x030000,
    LV_PART_SELECTED = 0x040000,
    LV_PART_ITEMS = 0x050000,
    LV_PART_TICKS = 0x060000,
    LV_PART_CURSOR = 0x070000,
    LV_PART_CUSTOM_FIRST = 0x080000,
    LV_PART_ANY = 0x0F0000,
} lv_part_t;

/* Alignment */
typedef enum {
    LV_ALIGN_DEFAULT = 0,
    LV_ALIGN_TOP_LEFT,
    LV_ALIGN_TOP_MID,
    LV_ALIGN_TOP_RIGHT,
    LV_ALIGN_BOTTOM_LEFT,
    LV_ALIGN_BOTTOM_MID,
    LV_ALIGN_BOTTOM_RIGHT,
    LV_ALIGN_LEFT_MID,
    LV_ALIGN_RIGHT_MID,
    LV_ALIGN_CENTER,
    LV_ALIGN_OUT_TOP_LEFT,
    LV_ALIGN_OUT_TOP_MID,
    LV_ALIGN_OUT_TOP_RIGHT,
    LV_ALIGN_OUT_BOTTOM_LEFT,
    LV_ALIGN_OUT_BOTTOM_MID,
    LV_ALIGN_OUT_BOTTOM_RIGHT,
    LV_ALIGN_OUT_LEFT_TOP,
    LV_ALIGN_OUT_LEFT_MID,
    LV_ALIGN_OUT_LEFT_BOTTOM,
    LV_ALIGN_OUT_RIGHT_TOP,
    LV_ALIGN_OUT_RIGHT_MID,
    LV_ALIGN_OUT_RIGHT_BOTTOM,
} lv_align_t;

/* Direction */
typedef enum {
    LV_DIR_NONE = 0x00,
    LV_DIR_LEFT = (1 << 0),
    LV_DIR_RIGHT = (1 << 1),
    LV_DIR_TOP = (1 << 2),
    LV_DIR_BOTTOM = (1 << 3),
    LV_DIR_HOR = LV_DIR_LEFT | LV_DIR_RIGHT,
    LV_DIR_VER = LV_DIR_TOP | LV_DIR_BOTTOM,
    LV_DIR_ALL = LV_DIR_HOR | LV_DIR_VER,
} lv_dir_t;

/* Size special values */
#define LV_SIZE_CONTENT 0x7FFF
#define LV_PCT(x) (((x) < 0 ? 0x8001 : 0x8000) | ((x) < 0 ? -(x) : (x)))

/* Object creation/deletion */
lv_obj_t* lv_obj_create(lv_obj_t* parent);
void lv_obj_del(lv_obj_t* obj);
void lv_obj_clean(lv_obj_t* obj);

/* Position and size */
void lv_obj_set_pos(lv_obj_t* obj, lv_coord_t x, lv_coord_t y);
void lv_obj_set_x(lv_obj_t* obj, lv_coord_t x);
void lv_obj_set_y(lv_obj_t* obj, lv_coord_t y);
void lv_obj_set_size(lv_obj_t* obj, lv_coord_t w, lv_coord_t h);
void lv_obj_set_width(lv_obj_t* obj, lv_coord_t w);
void lv_obj_set_height(lv_obj_t* obj, lv_coord_t h);
void lv_obj_set_align(lv_obj_t* obj, lv_align_t align);
void lv_obj_align(lv_obj_t* obj, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);
void lv_obj_align_to(lv_obj_t* obj, const lv_obj_t* base, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);
void lv_obj_center(lv_obj_t* obj);

lv_coord_t lv_obj_get_x(const lv_obj_t* obj);
lv_coord_t lv_obj_get_y(const lv_obj_t* obj);
lv_coord_t lv_obj_get_width(const lv_obj_t* obj);
lv_coord_t lv_obj_get_height(const lv_obj_t* obj);

/* Flags and state */
void lv_obj_add_flag(lv_obj_t* obj, lv_obj_flag_t f);
void lv_obj_clear_flag(lv_obj_t* obj, lv_obj_flag_t f);
bool lv_obj_has_flag(const lv_obj_t* obj, lv_obj_flag_t f);
void lv_obj_add_state(lv_obj_t* obj, lv_state_t state);
void lv_obj_clear_state(lv_obj_t* obj, lv_state_t state);
lv_state_t lv_obj_get_state(const lv_obj_t* obj);
bool lv_obj_has_state(const lv_obj_t* obj, lv_state_t state);

/* Events */
void lv_obj_add_event_cb(lv_obj_t* obj, lv_event_cb_t event_cb, lv_event_code_t filter, void* user_data);
bool lv_obj_remove_event_cb(lv_obj_t* obj, lv_event_cb_t event_cb);

/* User data */
void lv_obj_set_user_data(lv_obj_t* obj, void* user_data);
void* lv_obj_get_user_data(const lv_obj_t* obj);

/* Parent/child */
lv_obj_t* lv_obj_get_parent(const lv_obj_t* obj);
lv_obj_t* lv_obj_get_child(const lv_obj_t* obj, int32_t id);
uint32_t lv_obj_get_child_cnt(const lv_obj_t* obj);

/*====================
 * STYLE FUNCTIONS
 *====================*/

/* Style properties */
typedef enum {
    LV_STYLE_PROP_INV = 0,
    LV_STYLE_WIDTH,
    LV_STYLE_MIN_WIDTH,
    LV_STYLE_MAX_WIDTH,
    LV_STYLE_HEIGHT,
    LV_STYLE_MIN_HEIGHT,
    LV_STYLE_MAX_HEIGHT,
    LV_STYLE_X,
    LV_STYLE_Y,
    LV_STYLE_ALIGN,
    LV_STYLE_TRANSFORM_WIDTH,
    LV_STYLE_TRANSFORM_HEIGHT,
    LV_STYLE_TRANSLATE_X,
    LV_STYLE_TRANSLATE_Y,
    LV_STYLE_TRANSFORM_ZOOM,
    LV_STYLE_TRANSFORM_ANGLE,
    LV_STYLE_PAD_TOP,
    LV_STYLE_PAD_BOTTOM,
    LV_STYLE_PAD_LEFT,
    LV_STYLE_PAD_RIGHT,
    LV_STYLE_PAD_ROW,
    LV_STYLE_PAD_COLUMN,
    LV_STYLE_BG_COLOR,
    LV_STYLE_BG_OPA,
    LV_STYLE_BG_GRAD_COLOR,
    LV_STYLE_BG_GRAD_DIR,
    LV_STYLE_BG_MAIN_STOP,
    LV_STYLE_BG_GRAD_STOP,
    LV_STYLE_BORDER_COLOR,
    LV_STYLE_BORDER_OPA,
    LV_STYLE_BORDER_WIDTH,
    LV_STYLE_BORDER_SIDE,
    LV_STYLE_BORDER_POST,
    LV_STYLE_OUTLINE_WIDTH,
    LV_STYLE_OUTLINE_COLOR,
    LV_STYLE_OUTLINE_OPA,
    LV_STYLE_OUTLINE_PAD,
    LV_STYLE_SHADOW_WIDTH,
    LV_STYLE_SHADOW_OFS_X,
    LV_STYLE_SHADOW_OFS_Y,
    LV_STYLE_SHADOW_SPREAD,
    LV_STYLE_SHADOW_COLOR,
    LV_STYLE_SHADOW_OPA,
    LV_STYLE_IMG_OPA,
    LV_STYLE_IMG_RECOLOR,
    LV_STYLE_IMG_RECOLOR_OPA,
    LV_STYLE_LINE_WIDTH,
    LV_STYLE_LINE_DASH_WIDTH,
    LV_STYLE_LINE_DASH_GAP,
    LV_STYLE_LINE_ROUNDED,
    LV_STYLE_LINE_COLOR,
    LV_STYLE_LINE_OPA,
    LV_STYLE_ARC_WIDTH,
    LV_STYLE_ARC_ROUNDED,
    LV_STYLE_ARC_COLOR,
    LV_STYLE_ARC_OPA,
    LV_STYLE_ARC_IMG_SRC,
    LV_STYLE_TEXT_COLOR,
    LV_STYLE_TEXT_OPA,
    LV_STYLE_TEXT_FONT,
    LV_STYLE_TEXT_LETTER_SPACE,
    LV_STYLE_TEXT_LINE_SPACE,
    LV_STYLE_TEXT_DECOR,
    LV_STYLE_TEXT_ALIGN,
    LV_STYLE_RADIUS,
    LV_STYLE_CLIP_CORNER,
    LV_STYLE_OPA,
    LV_STYLE_COLOR_FILTER_DSC,
    LV_STYLE_COLOR_FILTER_OPA,
    LV_STYLE_ANIM_TIME,
    LV_STYLE_ANIM_SPEED,
    LV_STYLE_TRANSITION,
    LV_STYLE_BLEND_MODE,
    LV_STYLE_LAYOUT,
    LV_STYLE_BASE_DIR,
} lv_style_prop_t;

/* Style functions */
void lv_style_init(lv_style_t* style);
void lv_style_reset(lv_style_t* style);
void lv_style_set_width(lv_style_t* style, lv_coord_t value);
void lv_style_set_height(lv_style_t* style, lv_coord_t value);
void lv_style_set_bg_color(lv_style_t* style, lv_color_t color);
void lv_style_set_bg_opa(lv_style_t* style, lv_opa_t value);
void lv_style_set_text_color(lv_style_t* style, lv_color_t color);
void lv_style_set_text_font(lv_style_t* style, const lv_font_t* font);
void lv_style_set_border_width(lv_style_t* style, lv_coord_t value);
void lv_style_set_border_color(lv_style_t* style, lv_color_t color);
void lv_style_set_radius(lv_style_t* style, lv_coord_t value);
void lv_style_set_pad_all(lv_style_t* style, lv_coord_t value);
void lv_style_set_pad_top(lv_style_t* style, lv_coord_t value);
void lv_style_set_pad_bottom(lv_style_t* style, lv_coord_t value);
void lv_style_set_pad_left(lv_style_t* style, lv_coord_t value);
void lv_style_set_pad_right(lv_style_t* style, lv_coord_t value);

/* Style selector helper */
typedef uint32_t lv_style_selector_t;

void lv_obj_add_style(lv_obj_t* obj, lv_style_t* style, lv_style_selector_t selector);
void lv_obj_remove_style(lv_obj_t* obj, lv_style_t* style, lv_style_selector_t selector);
void lv_obj_remove_style_all(lv_obj_t* obj);

/* Local style setters */
void lv_obj_set_style_bg_color(lv_obj_t* obj, lv_color_t color, lv_style_selector_t selector);
void lv_obj_set_style_bg_opa(lv_obj_t* obj, lv_opa_t value, lv_style_selector_t selector);
void lv_obj_set_style_text_color(lv_obj_t* obj, lv_color_t color, lv_style_selector_t selector);
void lv_obj_set_style_border_width(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector);
void lv_obj_set_style_border_color(lv_obj_t* obj, lv_color_t color, lv_style_selector_t selector);
void lv_obj_set_style_radius(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector);
void lv_obj_set_style_pad_all(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector);
void lv_obj_set_style_pad_top(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector);
void lv_obj_set_style_pad_bottom(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector);
void lv_obj_set_style_pad_left(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector);
void lv_obj_set_style_pad_right(lv_obj_t* obj, lv_coord_t value, lv_style_selector_t selector);

/*====================
 * WIDGET: BUTTON
 *====================*/

lv_obj_t* lv_btn_create(lv_obj_t* parent);

/*====================
 * WIDGET: LABEL
 *====================*/

/* Text alignment for labels */
typedef enum {
    LV_TEXT_ALIGN_AUTO,
    LV_TEXT_ALIGN_LEFT,
    LV_TEXT_ALIGN_CENTER,
    LV_TEXT_ALIGN_RIGHT,
} lv_text_align_t;

/* Long mode */
typedef enum {
    LV_LABEL_LONG_WRAP,
    LV_LABEL_LONG_DOT,
    LV_LABEL_LONG_SCROLL,
    LV_LABEL_LONG_SCROLL_CIRCULAR,
    LV_LABEL_LONG_CLIP,
} lv_label_long_mode_t;

lv_obj_t* lv_label_create(lv_obj_t* parent);
void lv_label_set_text(lv_obj_t* obj, const char* txt);
void lv_label_set_text_fmt(lv_obj_t* obj, const char* fmt, ...);
void lv_label_set_text_static(lv_obj_t* obj, const char* txt);
void lv_label_set_long_mode(lv_obj_t* obj, lv_label_long_mode_t mode);
void lv_label_set_recolor(lv_obj_t* obj, bool en);
const char* lv_label_get_text(const lv_obj_t* obj);
lv_label_long_mode_t lv_label_get_long_mode(const lv_obj_t* obj);

/*====================
 * WIDGET: TABVIEW
 *====================*/

lv_obj_t* lv_tabview_create(lv_obj_t* parent, lv_dir_t tab_pos, lv_coord_t tab_size);
lv_obj_t* lv_tabview_add_tab(lv_obj_t* tv, const char* name);
void lv_tabview_set_act(lv_obj_t* tv, uint32_t id, int anim_type);
uint16_t lv_tabview_get_tab_act(lv_obj_t* tv);
lv_obj_t* lv_tabview_get_content(lv_obj_t* tv);
lv_obj_t* lv_tabview_get_tab_btns(lv_obj_t* tv);

/*====================
 * WIDGET: BUTTON MATRIX
 *====================*/

typedef enum {
    LV_BTNMATRIX_CTRL_HIDDEN = 0x0008,
    LV_BTNMATRIX_CTRL_NO_REPEAT = 0x0010,
    LV_BTNMATRIX_CTRL_DISABLED = 0x0020,
    LV_BTNMATRIX_CTRL_CHECKABLE = 0x0040,
    LV_BTNMATRIX_CTRL_CHECKED = 0x0080,
    LV_BTNMATRIX_CTRL_CLICK_TRIG = 0x0100,
    LV_BTNMATRIX_CTRL_POPOVER = 0x0200,
    LV_BTNMATRIX_CTRL_RECOLOR = 0x1000,
    LV_BTNMATRIX_CTRL_CUSTOM_1 = 0x4000,
    LV_BTNMATRIX_CTRL_CUSTOM_2 = 0x8000,
} lv_btnmatrix_ctrl_t;

lv_obj_t* lv_btnmatrix_create(lv_obj_t* parent);
void lv_btnmatrix_set_map(lv_obj_t* obj, const char* map[]);
void lv_btnmatrix_set_ctrl_map(lv_obj_t* obj, const lv_btnmatrix_ctrl_t ctrl_map[]);
void lv_btnmatrix_set_btn_ctrl(lv_obj_t* obj, uint16_t btn_id, lv_btnmatrix_ctrl_t ctrl);
void lv_btnmatrix_clear_btn_ctrl(lv_obj_t* obj, uint16_t btn_id, lv_btnmatrix_ctrl_t ctrl);
void lv_btnmatrix_set_btn_ctrl_all(lv_obj_t* obj, lv_btnmatrix_ctrl_t ctrl);
void lv_btnmatrix_clear_btn_ctrl_all(lv_obj_t* obj, lv_btnmatrix_ctrl_t ctrl);
void lv_btnmatrix_set_one_checked(lv_obj_t* obj, bool en);
uint16_t lv_btnmatrix_get_selected_btn(const lv_obj_t* obj);
const char* lv_btnmatrix_get_btn_text(const lv_obj_t* obj, uint16_t btn_id);
bool lv_btnmatrix_has_btn_ctrl(const lv_obj_t* obj, uint16_t btn_id, lv_btnmatrix_ctrl_t ctrl);

/*====================
 * WIDGET: BAR
 *====================*/

lv_obj_t* lv_bar_create(lv_obj_t* parent);
void lv_bar_set_value(lv_obj_t* obj, int32_t value, int anim);
void lv_bar_set_range(lv_obj_t* obj, int32_t min, int32_t max);
int32_t lv_bar_get_value(const lv_obj_t* obj);
int32_t lv_bar_get_min_value(const lv_obj_t* obj);
int32_t lv_bar_get_max_value(const lv_obj_t* obj);

/*====================
 * WIDGET: SLIDER
 *====================*/

lv_obj_t* lv_slider_create(lv_obj_t* parent);
void lv_slider_set_value(lv_obj_t* obj, int32_t value, int anim);
void lv_slider_set_range(lv_obj_t* obj, int32_t min, int32_t max);
int32_t lv_slider_get_value(const lv_obj_t* obj);
int32_t lv_slider_get_min_value(const lv_obj_t* obj);
int32_t lv_slider_get_max_value(const lv_obj_t* obj);

/*====================
 * WIDGET: SWITCH
 *====================*/

lv_obj_t* lv_switch_create(lv_obj_t* parent);

/*====================
 * WIDGET: CHECKBOX
 *====================*/

lv_obj_t* lv_checkbox_create(lv_obj_t* parent);
void lv_checkbox_set_text(lv_obj_t* obj, const char* txt);
void lv_checkbox_set_text_static(lv_obj_t* obj, const char* txt);
const char* lv_checkbox_get_text(const lv_obj_t* obj);

/*====================
 * WIDGET: DROPDOWN
 *====================*/

lv_obj_t* lv_dropdown_create(lv_obj_t* parent);
void lv_dropdown_set_options(lv_obj_t* obj, const char* options);
void lv_dropdown_set_selected(lv_obj_t* obj, uint16_t sel_opt);
uint16_t lv_dropdown_get_selected(const lv_obj_t* obj);
void lv_dropdown_get_selected_str(const lv_obj_t* obj, char* buf, uint32_t buf_size);

/*====================
 * WIDGET: ROLLER
 *====================*/

typedef enum {
    LV_ROLLER_MODE_NORMAL,
    LV_ROLLER_MODE_INFINITE,
} lv_roller_mode_t;

lv_obj_t* lv_roller_create(lv_obj_t* parent);
void lv_roller_set_options(lv_obj_t* obj, const char* options, lv_roller_mode_t mode);
void lv_roller_set_selected(lv_obj_t* obj, uint16_t sel_opt, int anim);
uint16_t lv_roller_get_selected(const lv_obj_t* obj);
void lv_roller_get_selected_str(const lv_obj_t* obj, char* buf, uint32_t buf_size);

/*====================
 * WIDGET: TEXTAREA
 *====================*/

lv_obj_t* lv_textarea_create(lv_obj_t* parent);
void lv_textarea_add_char(lv_obj_t* obj, uint32_t c);
void lv_textarea_add_text(lv_obj_t* obj, const char* txt);
void lv_textarea_del_char(lv_obj_t* obj);
void lv_textarea_del_char_forward(lv_obj_t* obj);
void lv_textarea_set_text(lv_obj_t* obj, const char* txt);
void lv_textarea_set_placeholder_text(lv_obj_t* obj, const char* txt);
const char* lv_textarea_get_text(const lv_obj_t* obj);

/*====================
 * WIDGET: TABLE
 *====================*/

lv_obj_t* lv_table_create(lv_obj_t* parent);
void lv_table_set_cell_value(lv_obj_t* obj, uint16_t row, uint16_t col, const char* txt);
void lv_table_set_row_cnt(lv_obj_t* obj, uint16_t row_cnt);
void lv_table_set_col_cnt(lv_obj_t* obj, uint16_t col_cnt);
void lv_table_set_col_width(lv_obj_t* obj, uint16_t col_id, lv_coord_t w);
uint16_t lv_table_get_row_cnt(const lv_obj_t* obj);
uint16_t lv_table_get_col_cnt(const lv_obj_t* obj);
const char* lv_table_get_cell_value(const lv_obj_t* obj, uint16_t row, uint16_t col);

/*====================
 * WIDGET: LIST
 *====================*/

lv_obj_t* lv_list_create(lv_obj_t* parent);
lv_obj_t* lv_list_add_text(lv_obj_t* list, const char* txt);
lv_obj_t* lv_list_add_btn(lv_obj_t* list, const void* icon, const char* txt);
const char* lv_list_get_btn_text(const lv_obj_t* list, const lv_obj_t* btn);

/*====================
 * WIDGET: MESSAGE BOX
 *====================*/

lv_obj_t* lv_msgbox_create(lv_obj_t* parent, const char* title, const char* txt, const char* btn_txts[], bool add_close_btn);
lv_obj_t* lv_msgbox_get_btns(lv_obj_t* mbox);
const char* lv_msgbox_get_active_btn_text(lv_obj_t* mbox);
void lv_msgbox_close(lv_obj_t* mbox);

/*====================
 * WIDGET: SPINNER
 *====================*/

lv_obj_t* lv_spinner_create(lv_obj_t* parent, uint32_t time, uint32_t arc_length);

/*====================
 * WIDGET: CHART
 *====================*/

typedef enum {
    LV_CHART_TYPE_NONE,
    LV_CHART_TYPE_LINE,
    LV_CHART_TYPE_BAR,
    LV_CHART_TYPE_SCATTER,
} lv_chart_type_t;

typedef enum {
    LV_CHART_UPDATE_MODE_SHIFT,
    LV_CHART_UPDATE_MODE_CIRCULAR,
} lv_chart_update_mode_t;

typedef enum {
    LV_CHART_AXIS_PRIMARY_Y,
    LV_CHART_AXIS_SECONDARY_Y,
    LV_CHART_AXIS_PRIMARY_X,
    LV_CHART_AXIS_SECONDARY_X,
} lv_chart_axis_t;

typedef struct {
    lv_coord_t* x_points;
    lv_coord_t* y_points;
    lv_color_t color;
    uint16_t start_point;
    bool hidden;
    lv_coord_t x_ext_buf_assigned : 1;
    lv_coord_t y_ext_buf_assigned : 1;
    lv_coord_t y_axis_sec : 1;
    lv_coord_t x_axis_sec : 1;
} lv_chart_series_t;

lv_obj_t* lv_chart_create(lv_obj_t* parent);
void lv_chart_set_type(lv_obj_t* obj, lv_chart_type_t type);
void lv_chart_set_point_count(lv_obj_t* obj, uint16_t cnt);
void lv_chart_set_range(lv_obj_t* obj, lv_chart_axis_t axis, lv_coord_t min, lv_coord_t max);
void lv_chart_set_update_mode(lv_obj_t* obj, lv_chart_update_mode_t update_mode);
lv_chart_series_t* lv_chart_add_series(lv_obj_t* obj, lv_color_t color, lv_chart_axis_t axis);
void lv_chart_remove_series(lv_obj_t* obj, lv_chart_series_t* series);
void lv_chart_set_next_value(lv_obj_t* obj, lv_chart_series_t* ser, lv_coord_t value);
void lv_chart_refresh(lv_obj_t* obj);

/*====================
 * WIDGET: LED
 *====================*/

lv_obj_t* lv_led_create(lv_obj_t* parent);
void lv_led_set_color(lv_obj_t* obj, lv_color_t color);
void lv_led_set_brightness(lv_obj_t* obj, uint8_t bright);
void lv_led_on(lv_obj_t* obj);
void lv_led_off(lv_obj_t* obj);
void lv_led_toggle(lv_obj_t* obj);
uint8_t lv_led_get_brightness(const lv_obj_t* obj);

/*====================
 * ANIMATION
 *====================*/

#define LV_ANIM_OFF 0
#define LV_ANIM_ON 1

/*====================
 * LOG LEVELS
 *====================*/

#define LV_LOG_LEVEL_TRACE 0
#define LV_LOG_LEVEL_INFO 1
#define LV_LOG_LEVEL_WARN 2
#define LV_LOG_LEVEL_ERROR 3
#define LV_LOG_LEVEL_USER 4
#define LV_LOG_LEVEL_NONE 5

#ifdef __cplusplus
}
#endif

#endif /* LVGL_H */
