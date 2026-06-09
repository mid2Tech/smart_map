#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <lvgl.h>

// --- CẤU HÌNH CHÂN PHẦN CỨNG ---
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST 4
#define TFT_LED 21
// #define PIN_SWITCH 32
#define PIN_BUZZER 26
#define PIN_BATTERY 34
#define PIN_BTN_NEXT 33
#define PIN_BTN_PREV 25

extern int currentMapIndex;
extern int totalMaps;
extern bool systemOn;
extern bool isCheckedIn[15];
extern lv_obj_t *battery_label;

#endif