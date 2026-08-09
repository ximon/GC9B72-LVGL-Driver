/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>
#include "lv_gc9b72.h"

#if LV_USE_GC9B72

/*********************
 *      DEFINES
 *********************/

/* Inter-register enable commands, needed to unlock the vendor-specific
 * (undocumented) register range 0x60-0x99, 0xAA-0xC9, 0xEB-0xF9. */
#define CMD_INREGEN1 0xFE
#define CMD_UNLOCK 0xEF
#define CMD_LOCK 0xEE

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC CONSTANTS
 **********************/

/* Panel bring-up sequence, ported from the working Arduino_GC9B72 */
static const uint8_t init_cmd_list[] = {
    CMD_INREGEN1, 0,
    CMD_UNLOCK, 0,

    0x80, 1, 0x19,
    0x82, 1, 0x09,
    0x83, 1, 0x03,
    0x88, 1, 0x00,
    0x89, 1, 0x38,
    0x8A, 1, 0x40,
    0x8B, 1, 0x0A,
    0x8C, 1, 0x00,

    /* OTP */
    0x81, 1, 0xFF,
    0x84, 1, 0xFF,
    0x85, 1, 0xFF,
    0x86, 1, 0xFF,
    0x87, 1, 0xFF,
    0x8E, 1, 0xFF,
    0x8F, 1, 0xFF,

    /* BVDDEE */
    0x98, 1, 0x3E,
    0x99, 1, 0x3E,

    /* VGH */
    0x7D, 1, 0x72,

    /* CHP CLK */
    0x70, 10, 0x02, 0x03, 0x03, 0x06, 0x03, 0x03, 0x09, 0x07, 0x09, 0x03,

    /* BVDDEE CLK */
    0x90, 4, 0x06, 0x06, 0x01, 0x01,
    0x93, 3, 0x02, 0xFF, 0x00,
    0xCB, 1, 0x02,

    /* Source Chop */
    0xFB, 2, 0x00, 0x00,

    0xF6, 1, 0xC0,

    /* Gate */
    0x6C, 7, 0x00, 0x00, 0x22, 0x00, 0xCC, 0x04, 0x58,
    0xAA, 2, 0x0B, 0x00,

    0xEC, 1, 0x07, /* fw:07 bw:87 */

    /* Source */
    0xF9, 1, 0x40,

    /* Gate */
    0xEB, 2, 0x01, 0x67,

    /* RTN */
    0x74, 6, 0x01/*160*/, 0x60, 0x00, 0x00, 0x00, 0x00,

    /* VBP & VFP & HBP */
    0xB5, 3, 0x14, 0x14, 0x14,

    /* Mapping */
    0x6E, 32,
    0x0B, 0x0B, 0x09, 0x09, 0x13, 0x13, 0x11, 0x11,
    0x16/*15 gout9*/, 0x15/*16 gout10*/, 0x01, 0x04, 0x00, 0x0D, 0x1D, 0x00,
    0x00, 0x1D, 0x0D, 0x00, 0x04, 0x08, 0x15/*16 gout23*/, 0x16 /*15 gout24*/,
    0x12, 0x12, 0x14, 0x14, 0x0A, 0x0A, 0x0C, 0x0C,

     /* STV1 */
    0x60, 4, 0x38, 0x1C, 0x13, 0x56,
    /* STV4 */
    0x61, 4, 0xF8, 0x0A, 0x13, 0x56,
    /* STV5 */
    0x62, 4, 0xF8, 0x0B, 0x13, 0x56, 
    /* STV8 */
    0x63, 4, 0x38, 0x1C, 0x13, 0x56, 
 
    0x64, 6, 0x38, 0x20/*1E*/, 0x72, 0xF8, 0x13, 0x56, /* CLK1->panel_CLK5 */
    0x65, 6, 0x78, 0x1A, 0x70, 0x0B, 0x56, 0x13, /* CLK5->panel_VDS */
    0x66, 6, 0x38, 0x24/*22*/, 0x72, 0xFC, 0x13, 0x56, /* CLK9->panel_CLK1 */
 
    /* FLC12 */
    0x68, 7, 0xB3, 0x08/*2F0*/, 0x0E, 0x08, 0x0E, 0x0A, 0x0A, 
    
    /* FLC34 */
    0x69, 7, 0xB3, 0x08/*2F0*/, 0x0E, 0x08, 0x0E, 0x0A, 0x0A, 
    
    0x6A, 2, 0x00, 0x00,

    0x3A, 1, 0x05, /* pixel format, COLMOD: 16 bits per pixel RGB565 */
    0x36, 1, 0x05, /* MADCTL */ 
    /*
    this seems correct so far...
    0000 0000 00 = Normal 
    1001 0000 90 = Normal

    0 00 1 0000 10 = Mirror Y
    0 01 0 0000 20 = Mirror Y + Rotated 90 CCW 
    0 01 1 0000 30 = Rotated 90 CCW
    0 10 0 0000 40 = Mirror X
    0 10 1 0000 50 = Rotated 180
    0 11 0 0000 60 = Rotated 90 CW
    0 11 1 0000 70 = Mirror X + Rotated 90 CW
    
    1 00 0 0000 80 = Mirror Y
    1 00 1 0000 90 = Normal
    1 01 0 0000 A0 = Rotated 90 CCW
    1 01 1 0000 B0 = Mirror Y + Rotated 90 CCW
    1 10 0 0000 C0 = Rotated 180
    1 10 1 0000 D0 = Mirror X
    1 11 0 0000 E0 = Mirror X + Rotated 90 CW
    1 11 1 0000 F0 = Rotated 90 CW

    x xx x 1000 x8 = red = blue? (but green is still green) 
      1x             Mirror X
      0x             Mirror Y
      01             Rotate 90 CCW
      11             Rotate 90 CW
      10             Rotate 180
           1         BGR
           0         RGB
    */

    0x7C, 2, 0xB6, 0x29,

    /* VGL */
    0xAC, 1, 0x40,

    /* VREG */
    0xC3, 1, 0x1A,
    0xC4, 1, 0x24,
    0xC9, 1, 0x2F,

    /* Gamma */
    0xF0, 6, 0x11/*V62[5:0]*/, 0x17/*V61[5:0]*/, 0x08/*V59[4:0]*/, 0x06/*V57[4:0]*/, 0x05/*V63[7:4]/V50[3:0]*/, 0x38/*V43[6:0]*/,
    0xF1, 6, 0x4D/*V20[6:0]*/, 0x72/*V36[7:5]/V6[4:0]*/, 0x72/*V27[7:5]/V4[4:0]*/, 0x2D/*V2[5:0]*/, 0x34/*V1[5:0]*/, 0x8F/*V13[7:4]/V0[3:0]*/,
    0xF2, 6, 0x11/*V62[5:0]*/, 0x17/*V61[5:0]*/, 0x08/*V59[4:0]*/, 0x06/*V57[4:0]*/, 0x05/*V63[7:4]/V50[3:0]*/, 0x38/*V43[6:0]*/,
    0xF3, 6, 0x4D/*V20[6:0]*/, 0x72/*V36[7:5]/V6[4:0]*/, 0x72/*V27[7:5]/V4[4:0]*/, 0x2D/*V2[5:0]*/, 0x34/*V1[5:0]*/, 0x8F/*V13[7:4]/V0[3:0]*/,

    /* TE pulse width */
    0xB4, 1, 0x0A,

    /* Tearing Effect line ON. Drives the panel's dedicated TE pin - leave
     * that pin unconnected/unused elsewhere or this will contend with it. */
    LV_LCD_CMD_SET_TEAR_ON, 1, 0x00,

    /* Lock registers back up */
    CMD_INREGEN1, 0,
    CMD_LOCK, 0,

    /* Exit Sleep Mode */
    //LV_LCD_CMD_EXIT_SLEEP_MODE, 0,
    LV_LCD_CMD_ENTER_SLEEP_MODE, 0,
    LV_LCD_CMD_DELAY_MS, 12/*120ms */,

    /* Turn Display On */
    //LV_LCD_CMD_SET_DISPLAY_ON, 0,
    LV_LCD_CMD_SET_DISPLAY_OFF, 0,
    LV_LCD_CMD_DELAY_MS, 2/*20ms*/,

    /* terminate list: required */
    LV_LCD_CMD_DELAY_MS, LV_LCD_CMD_EOF 
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_display_t * lv_gc9b72_create(uint32_t hor_res, uint32_t ver_res, lv_lcd_flag_t flags,
                                 lv_gc9b72_send_cmd_cb_t send_cmd_cb, lv_gc9b72_send_color_cb_t send_color_cb)
{
    lv_display_t * disp = lv_lcd_generic_mipi_create(hor_res, ver_res, flags, send_cmd_cb, send_color_cb);
    lv_lcd_generic_mipi_send_cmd_list(disp, init_cmd_list);

    return disp;
}

void lv_gc9b72_set_gap(lv_display_t * disp, uint16_t x, uint16_t y)
{
    lv_lcd_generic_mipi_set_gap(disp, x, y);
}

void lv_gc9b72_set_invert(lv_display_t * disp, bool invert)
{
    lv_lcd_generic_mipi_set_invert(disp, invert);
}

void lv_gc9b72_set_gamma_curve(lv_display_t * disp, uint8_t gamma)
{
    lv_lcd_generic_mipi_set_gamma_curve(disp, gamma);
}

void lv_gc9b72_send_cmd_list(lv_display_t * disp, const uint8_t * cmd_list)
{
    lv_lcd_generic_mipi_send_cmd_list(disp, cmd_list);
}

void lv_gc9b72_send_display_on(lv_display_t * disp)
{
    static const uint8_t cmd_list[] = {
        LV_LCD_CMD_EXIT_SLEEP_MODE, 0,
        LV_LCD_CMD_DELAY_MS, 12,//120ms
     
        LV_LCD_CMD_SET_DISPLAY_ON, 0,
        LV_LCD_CMD_DELAY_MS, 2, //20ms
     
        LV_LCD_CMD_DELAY_MS, LV_LCD_CMD_EOF
    };
    lv_lcd_generic_mipi_send_cmd_list(disp, cmd_list);
}

void lv_gc9b72_send_display_off(lv_display_t * disp)
{
    static const uint8_t cmd_list[] = {
        LV_LCD_CMD_SET_DISPLAY_OFF, 0,
        LV_LCD_CMD_DELAY_MS, 2, //20ms
     
        LV_LCD_CMD_ENTER_SLEEP_MODE, 0,
        LV_LCD_CMD_DELAY_MS, 12, //120ms
     
        LV_LCD_CMD_DELAY_MS, LV_LCD_CMD_EOF
    };
    lv_lcd_generic_mipi_send_cmd_list(disp, cmd_list);
}

void lv_gc9b72_blank(lv_display_t * disp)
{
  //send black screen
  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  lv_timer_handler(); //force refresh
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_GC9B72*/