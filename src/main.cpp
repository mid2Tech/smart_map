#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <lvgl.h>
#include "ui/ui.h"

#include "Globals.h"
#include "ui/UI_Manager.h"
#include "ble/BLE_Manager.h"

int currentMapIndex = 1;
int totalMaps = 2;
bool isCheckedIn[15] = {false};
lv_obj_t *battery_label;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[240 * 10];
unsigned long lastTick = 0;
unsigned long lastBatteryUpdate = 0;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

volatile bool flagNextMap = false;
volatile bool flagPrevMap = false;
volatile unsigned long lastBtnInterrupt = 0;

void IRAM_ATTR isrNextMap()
{
  unsigned long currentMillis = millis();
  if (currentMillis - lastBtnInterrupt > 300)
  {
    flagNextMap = true;
    lastBtnInterrupt = currentMillis;
  }
}

void IRAM_ATTR isrPrevMap()
{
  unsigned long currentMillis = millis();
  if (currentMillis - lastBtnInterrupt > 300)
  {
    flagPrevMap = true;
    lastBtnInterrupt = currentMillis;
  }
}

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.drawRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp_drv);
}

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
  pinMode(PIN_BTN_PREV, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_NEXT), isrNextMap, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_PREV), isrPrevMap, FALLING);

  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  pinMode(PIN_BATTERY, INPUT);

  tft.init(240, 320);
  tft.invertDisplay(false);
  tft.setRotation(2);
  tft.fillScreen(0x0000);
  tft.sendCommand(0x11);
  delay(120);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 240 * 10);
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 240;
  disp_drv.ver_res = 320;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  ui_init();

  battery_label = lv_label_create(lv_layer_top());
  lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -5, 2);
  lv_obj_set_style_text_color(battery_label, lv_color_hex(0x000000), 0);
  lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_FULL " 100%");

  initUiToGray();
  switchMapAndText(1);

  setupBLE();
}

void loop()
{
  unsigned long currentMillis = millis();

  if (flagNextMap)
  {
    flagNextMap = false;
    currentMapIndex++;
    if (currentMapIndex > totalMaps)
      currentMapIndex = 1;
    switchMapAndText(currentMapIndex);
    Serial.print("Chuyen sang Map: ");
    Serial.println(currentMapIndex);
  }
  if (flagPrevMap)
  {
    flagPrevMap = false;
    currentMapIndex--;
    if (currentMapIndex < 1)
      currentMapIndex = totalMaps;
    switchMapAndText(currentMapIndex);
    Serial.print("Lui ve Map: ");
    Serial.println(currentMapIndex);
  }

  lv_tick_inc(currentMillis - lastTick);
  lastTick = currentMillis;
  lv_timer_handler();

  scanAndProcessBLE();

  if (currentMillis - lastBatteryUpdate > 2000)
  {
    long sumAnalog = 0;
    for (int i = 0; i < 20; i++)
    {
      sumAnalog += analogRead(PIN_BATTERY);
      delay(2);
    }
    int rawVal = sumAnalog / 20;

    float calibration_factor = 6.6;
    float voltage = (rawVal / 4095.0) * calibration_factor;

    int batPercent = 0;
    if (voltage >= 4.15)
      batPercent = 100;
    else if (voltage >= 4.00)
      batPercent = 80 + (int)((voltage - 4.00) / 0.15 * 20);
    else if (voltage >= 3.80)
      batPercent = 40 + (int)((voltage - 3.80) / 0.20 * 40);
    else if (voltage >= 3.60)
      batPercent = 10 + (int)((voltage - 3.60) / 0.20 * 30);
    else if (voltage >= 3.30)
      batPercent = (int)((voltage - 3.30) / 0.30 * 10);
    else
      batPercent = 0;

    if (batPercent > 100)
      batPercent = 100;
    if (batPercent < 0)
      batPercent = 0;

    update_battery_display(batPercent);
    lastBatteryUpdate = millis();
  }
}