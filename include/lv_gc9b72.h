#ifndef LV_GC9B72_H
#define LV_GC9B72_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "..\.pio\libdeps\esp32-s3-devkitm-1\lvgl\src\drivers\display\lcd\lv_lcd_generic_mipi.h"

#if LV_USE_GC9B72

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef lv_lcd_send_cmd_cb_t lv_gc9b72_send_cmd_cb_t;
typedef lv_lcd_send_color_cb_t lv_gc9b72_send_color_cb_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an LCD display with GC9B72 driver
 * @param hor_res       horizontal resolution
 * @param ver_res       vertical resolution
 * @param flags         default configuration settings (mirror, RGB ordering, etc.)
 * @param send_cmd_cb   platform-dependent function to send a command to the LCD controller (usually uses polling transfer)
 * @param send_color_cb platform-dependent function to send pixel data to the LCD controller (usually uses DMA transfer: must implement a 'ready' callback)
 * @return pointer to the created display
 */
lv_display_t * lv_gc9b72_create(uint32_t hor_res, uint32_t ver_res, lv_lcd_flag_t flags,
                                 lv_gc9b72_send_cmd_cb_t send_cmd_cb, lv_gc9b72_send_color_cb_t send_color_cb);

/**
 * Set gap, i.e., the offset of the (0,0) pixel in the VRAM
 * @param disp display object
 * @param x     x offset
 * @param y     y offset
 */
void lv_gc9b72_set_gap(lv_display_t * disp, uint16_t x, uint16_t y);

/**
 * Set color inversion
 * @param disp   display object
 * @param invert false: normal, true: invert
 */
void lv_gc9b72_set_invert(lv_display_t * disp, bool invert);

/**
 * Set gamma curve
 * @param disp  display object
 * @param gamma gamma curve
 */
void lv_gc9b72_set_gamma_curve(lv_display_t * disp, uint8_t gamma);

/**
 * Send list of commands.
 * @param disp     display object
 * @param cmd_list controller and panel-specific commands
 */
void lv_gc9b72_send_cmd_list(lv_display_t * disp, const uint8_t * cmd_list);

/**
 * Turn the display on.
 * 
 * @param disp display object
 */
void lv_gc9b72_send_display_on(lv_display_t * disp);

/**
 * @Turn the display off.
 * 
 * @param disp display object
 */
void lv_gc9b72_send_display_off(lv_display_t * disp);

/**
 * @brief Send a blank frame to the display
 * 
 * @param disp display object
 */
void lv_gc9b72_blank(lv_display_t * disp);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GC9B72*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GC9B72_H*/