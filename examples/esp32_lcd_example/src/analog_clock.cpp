#include "analog_clock.h"
#include <math.h>

typedef struct {
    lv_obj_t * hour_line;
    lv_obj_t * min_line;
    lv_obj_t * sec_line;
    lv_point_precise_t hour_pts[2];
    lv_point_precise_t min_pts[2];
    lv_point_precise_t sec_pts[2];
    lv_coord_t cx, cy;
    uint8_t h, m, s;
} clock_data_t;

static void set_hand(lv_obj_t * line, lv_point_precise_t * pts, lv_coord_t cx, lv_coord_t cy,
                      lv_coord_t len, float angle_deg) {
    float rad = (angle_deg - 90) * (float)M_PI / 180.0f;
    pts[0].x = cx;
    pts[0].y = cy;
    pts[1].x = cx + (lv_coord_t)(len * cosf(rad));
    pts[1].y = cy + (lv_coord_t)(len * sinf(rad));
    lv_line_set_points(line, pts, 2);
}

static void clock_update(lv_obj_t * clock) {
    clock_data_t * d = (clock_data_t *)lv_obj_get_user_data(clock);

    float sec_deg  = d->s * 6.0f;
    float min_deg  = d->m * 6.0f + d->s * 0.1f;
    float hour_deg = (d->h % 12) * 30.0f + d->m * 0.5f;

    set_hand(d->hour_line, d->hour_pts, d->cx, d->cy, d->cy * 0.5f, hour_deg);
    set_hand(d->min_line,  d->min_pts,  d->cx, d->cy, d->cy * 0.7f, min_deg);
    set_hand(d->sec_line,  d->sec_pts,  d->cx, d->cy, d->cy * 0.8f, sec_deg);
}

static void demo_timer_cb(lv_timer_t * t) {
    lv_obj_t * clock = (lv_obj_t *)lv_timer_get_user_data(t);
    clock_data_t * d = (clock_data_t *)lv_obj_get_user_data(clock);

    d->s++;
    if(d->s >= 60) { d->s = 0; d->m++; }
    if(d->m >= 60) { d->m = 0; d->h++; }
    if(d->h >= 24) d->h = 0;

    clock_update(clock);
}

lv_obj_t * analog_clock_create(lv_obj_t * parent, lv_coord_t size) {
    lv_obj_t * clock = lv_obj_create(parent);
    lv_obj_set_size(clock, size, size);
    lv_obj_set_style_radius(clock, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(clock, 0, 0);
    lv_obj_remove_flag(clock, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * scale = lv_scale_create(clock);
    lv_obj_set_size(scale, size, size);
    lv_obj_center(scale);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_range(scale, 0, 60);
    lv_scale_set_total_tick_count(scale, 61);
    lv_scale_set_major_tick_every(scale, 5);
    lv_scale_set_angle_range(scale, 360);
    lv_scale_set_rotation(scale, 270);
    lv_scale_set_label_show(scale, false);
    lv_obj_set_style_length(scale, LV_MAX(2, (int)(size * 0.03f)), LV_PART_ITEMS);
    lv_obj_set_style_length(scale, LV_MAX(4, (int)(size * 0.06f)), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(scale, LV_OPA_TRANSP, 0);

    clock_data_t * d = (clock_data_t *)lv_malloc(sizeof(clock_data_t));
    lv_memzero(d, sizeof(clock_data_t));
    d->cx = size / 2;
    d->cy = size / 2;

    d->hour_line = lv_line_create(clock);
    lv_obj_set_style_line_width(d->hour_line, LV_MAX(2, (int)(size * 0.02f)), 0);
    lv_obj_set_style_line_rounded(d->hour_line, true, 0);

    d->min_line = lv_line_create(clock);
    lv_obj_set_style_line_width(d->min_line, LV_MAX(2, (int)(size * 0.013f)), 0);
    lv_obj_set_style_line_rounded(d->min_line, true, 0);

    d->sec_line = lv_line_create(clock);
    lv_obj_set_style_line_width(d->sec_line, LV_MAX(1, (int)(size * 0.007f)), 0);
    lv_obj_set_style_line_rounded(d->sec_line, true, 0);
    lv_obj_set_style_line_color(d->sec_line, lv_palette_main(LV_PALETTE_RED), 0);

    lv_obj_t * pin = lv_obj_create(clock);
    lv_coord_t pin_size = LV_MAX(4, (int)(size * 0.035f));
    lv_obj_set_size(pin, pin_size, pin_size);
    lv_obj_set_style_radius(pin, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pin, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_center(pin);

    lv_obj_set_user_data(clock, d);
    clock_update(clock);

    return clock;
}

void analog_clock_set_time(lv_obj_t * clock, uint8_t h, uint8_t m, uint8_t s) {
    clock_data_t * d = (clock_data_t *)lv_obj_get_user_data(clock);
    d->h = h;
    d->m = m;
    d->s = s;
    clock_update(clock);
}

void analog_clock_start_demo(lv_obj_t * clock) {
    lv_timer_create(demo_timer_cb, 1000, clock);
}