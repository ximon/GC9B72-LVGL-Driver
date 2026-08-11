#ifndef ANALOG_CLOCK_H
#define ANALOG_CLOCK_H

#include "lvgl.h"

lv_obj_t * analog_clock_create(lv_obj_t * parent, lv_coord_t size);
void analog_clock_set_time(lv_obj_t * clock, uint8_t h, uint8_t m, uint8_t s);
void analog_clock_start_demo(lv_obj_t * clock);

#endif