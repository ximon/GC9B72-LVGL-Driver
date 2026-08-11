#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "lv_gc9b72.h"
#include "analog_clock.h"


#define TFT_SCLK 12
#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
#define TFT_BL   14

#define TFT_W 360
#define TFT_H 360

static lv_display_t *disp;
static lv_color_t draw_buf1[TFT_W * 60];
static lv_color_t draw_buf2[TFT_W * 60];

// ---- low-level SPI helpers ---- 

static inline void cs_low()  { digitalWrite(TFT_CS, LOW); }
static inline void cs_high() { digitalWrite(TFT_CS, HIGH); }
static inline void dc_low()  { digitalWrite(TFT_DC, LOW); }
static inline void dc_high() { digitalWrite(TFT_DC, HIGH); }
static inline void rst_low()  { digitalWrite(TFT_RST, HIGH); }
static inline void rst_high() { digitalWrite(TFT_RST, HIGH); }
static inline void bl_low()   { digitalWrite(TFT_BL, LOW); }
static inline void bl_high()  { digitalWrite(TFT_BL, HIGH); }

static void spi_write_bytes(const uint8_t *data, size_t len) {
  SPI.writeBytes(data, len);
}

// ---- LVGL generic MIPI callbacks ---- 

static void gc9b72_send_cmd(lv_display_t *d, const uint8_t *cmd, size_t cmd_size,
                             const uint8_t *param, size_t param_size) {
  LV_UNUSED(d);
  SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));
  cs_low();

  dc_low();
  spi_write_bytes(cmd, cmd_size);

  if (param_size) {
    digitalWrite(TFT_DC, HIGH); // data
    spi_write_bytes(param, param_size);
  }

  cs_high();
  SPI.endTransaction();
}

static void gc9b72_send_color(lv_display_t *d, const uint8_t *cmd, size_t cmd_size,
                               uint8_t *param, size_t param_size) {
  SPI.beginTransaction(SPISettings(80000000, MSBFIRST, SPI_MODE0));
  cs_low();

  dc_low();
  spi_write_bytes(cmd, cmd_size);
  
  dc_high();
  spi_write_bytes(param, param_size);
  
  cs_high();
  SPI.endTransaction();

  lv_display_flush_ready(d);
}

// ---- reset pulse (must happen before lv_gc9b72_create) ----

static void hw_reset() {
  pinMode(TFT_RST, OUTPUT);
 
  rst_high();
  delay(120);
 
  rst_low();
  delay(120);
 
  rst_high();
  delay(120);
}

// ---- LVGL tick source ---- 

static uint32_t my_tick_get_cb(void) {
  return millis();
}


lv_obj_t * clockObj;
void setup() {
  digitalWrite(BUILTIN_LED, HIGH);

  Serial.begin(115200);
  
  pinMode(TFT_BL, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  bl_high();
  cs_high();

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  hw_reset();

  lv_init();
  lv_tick_set_cb(my_tick_get_cb);

  disp = lv_gc9b72_create(TFT_W, TFT_H, LV_LCD_FLAG_NONE, gc9b72_send_cmd, gc9b72_send_color);
  lv_display_set_buffers(disp, draw_buf1, draw_buf2, sizeof(draw_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565_SWAPPED);

  lv_theme_t *th = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true/*dark=true*/, LV_FONT_DEFAULT); 

  lv_gc9b72_blank(disp);
  lv_gc9b72_send_display_on(disp);

  
  clockObj = analog_clock_create(lv_screen_active(), 60);
  lv_obj_align(clockObj, LV_ALIGN_TOP_LEFT, 150, 270);               // anywhere
  analog_clock_set_time(clockObj, 12, 00, 0);
  analog_clock_start_demo(clockObj);
}

void loop() {
  lv_timer_handler();
  delay(1);
}