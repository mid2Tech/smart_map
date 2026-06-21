#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <lvgl.h>

// Các thư viện của dự án
#include "ui/ui.h"
#include "ui/screens.h"
#include "Globals.h"
#include "ui/UI_Manager.h"
#include "ble/BLE_Manager.h"
#include "Navigation.h"
#include "compass/Compass.h"
#include "locationFilter/LocationFilter.h"
#include "button/ButtonHandler.h"

// =======================================================
// CẤU HÌNH HỆ THỐNG VÀ BIẾN TOÀN CỤC
// =======================================================
#define GOC_BU_LA_BAN 105
LV_FONT_DECLARE(ui_font_roboto)

extern int detectedMajor;

int targetLocID = 2;
bool isNavigating = false;
#define GOC_BU_LA_BAN 105
LV_FONT_DECLARE(ui_font_roboto)

bool isCheckedIn[15] = {false};
int currentMapIndex = 2;
int totalMaps = 1;
bool systemOn = true;

extern int detectedMajor;

struct ToaDoMain
{
  int x;
  int y;
};
ToaDoMain cac_tram[] = {
    {0, 0},
    {182, 18}, // Trạm 1: Tuyển sinh
    {10, 112}, // Trạm 2: IT
    {40, 159}, // Trạm 3: Thư viện
    {220, 61}  // Trạm 4: WC
};

const char *ten_tram[] = {
    "Không xác định", "Tuyển Sinh", "Phòng IT", "Thư Viện", "Nhà WC"};

// =======================================================
// ĐỐI TƯỢNG ĐỒ HỌA
// =======================================================
lv_obj_t *kim_chi_nam;
lv_obj_t *status_panel;
lv_obj_t *dest_panel;
lv_obj_t *dest_label;
lv_obj_t *status_label;
static lv_point_t line_points[5];

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[240 * 10];
unsigned long lastTick = 0;

float current_smooth_x = -1;
float current_smooth_y = -1;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.drawRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp_drv);
}

// =======================================================
// HÀM HIỂN THỊ UI & VẼ MŨI TÊN CHÍNH
// =======================================================
void xu_ly_dan_duong()
{
  int currentLoc = get_filtered_location(detectedMajor);
  int destLoc = targetLocID;
  char buf_str[128];

  int offset_x = 8;
  int offset_y = 8 + 59;

  static bool dang_giu_man_hinh_den = false;
  static unsigned long thoi_gian_den = 0;

  // 1. CẬP NHẬT MÀU BẢN ĐỒ KHI LẠI GẦN
  static int last_ui_loc = -1;
  if (currentLoc > 0 && currentLoc != last_ui_loc)
  {
    resetMapDotsToGray();
    turnOnMapDot(currentLoc);
    checkInHistory(currentLoc);
    last_ui_loc = currentLoc;
  }

  // 2. TÍNH DELTA TIME & LA BÀN
  static unsigned long last_pdr_time = millis();
  unsigned long now = millis();
  float dt = (now - last_pdr_time) / 1000.0;
  last_pdr_time = now;

  static float smoothed_heading = -1;
  int raw_heading = getCompassHeading();

  if (raw_heading >= 0)
  {
    float adjusted_heading = raw_heading + GOC_BU_LA_BAN;
    while (adjusted_heading < 0)
      adjusted_heading += 360;
    while (adjusted_heading >= 360)
      adjusted_heading -= 360;

    if (adjusted_heading > 345 || adjusted_heading < 15)
      adjusted_heading = 0;

    if (smoothed_heading < 0)
      smoothed_heading = adjusted_heading;
    else
    {
      float diff = adjusted_heading - smoothed_heading;
      if (diff > 180)
        diff -= 360;
      if (diff < -180)
        diff += 360;
      smoothed_heading += diff * 0.15;
      if (smoothed_heading < 0)
        smoothed_heading += 360;
      if (smoothed_heading >= 360)
        smoothed_heading -= 360;
    }
  }

  float final_heading = (smoothed_heading >= 0) ? smoothed_heading : 0;
  float rad = final_heading * M_PI / 180.0;
  float s = sin(rad), c = cos(rad);

  // 3. TÍNH TOÁN QUỸ ĐẠO MŨI TÊN
  if (currentLoc > 0)
  {
    float target_x = cac_tram[currentLoc].x + offset_x;
    float target_y = cac_tram[currentLoc].y + offset_y;
    if (current_smooth_x < 0)
    {
      current_smooth_x = target_x;
      current_smooth_y = target_y;
    }
    else
    {
      current_smooth_x = current_smooth_x * 0.9 + target_x * 0.1;
      current_smooth_y = current_smooth_y * 0.9 + target_y * 0.1;
    }
  }

  if (isNavigating && current_smooth_x >= 0)
  {
    float walking_speed = 8.0;
    current_smooth_x += walking_speed * cos(rad - M_PI / 2) * dt;
    current_smooth_y += walking_speed * sin(rad - M_PI / 2) * dt;
  }

  // TRẠNG THÁI: Chạm đích đến
  if (isNavigating && currentLoc > 0 && currentLoc == destLoc)
  {
    isNavigating = false;
    dang_giu_man_hinh_den = true;
    thoi_gian_den = millis();
  }

  // TRẠNG THÁI: Hết 10 giây chờ
  if (dang_giu_man_hinh_den && (millis() - thoi_gian_den > 10000))
  {
    dang_giu_man_hinh_den = false;
  }

  // =======================================================
  // 4. QUẢN LÝ GIAO DIỆN (TÁCH 2 KHUNG ĐỘC LẬP)
  // =======================================================
  if (currentLoc == 0)
  {
    // Khi mất sóng: Chỉ hiện khung trên
    lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(status_label, "Đang tìm vị trí hiện tại...");
    lv_obj_add_flag(dest_panel, LV_OBJ_FLAG_HIDDEN); // Giấu khung dưới đi
    lv_obj_add_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  else if (dang_giu_man_hinh_den)
  {
    lv_obj_clear_flag(dest_panel, LV_OBJ_FLAG_HIDDEN); // Bật khung dưới

    // Khung 1 (Xanh lá)
    lv_obj_set_style_bg_color(status_panel, lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_border_color(status_panel, lv_color_hex(0x2E7D32), 0);
    lv_label_set_text(status_label, "ĐÃ ĐẾN NƠI!");

    // Khung 2 (Trắng)
    lv_obj_set_style_bg_color(dest_panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(dest_panel, lv_color_hex(0x4CAF50), 0);
    snprintf(buf_str, sizeof(buf_str), "Mục tiêu: %s", ten_tram[destLoc]);
    lv_label_set_text(dest_label, buf_str);

    lv_obj_clear_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);
  }
  else if (isNavigating)
  {
    lv_obj_clear_flag(dest_panel, LV_OBJ_FLAG_HIDDEN);

    // Khung 1: Báo vị trí (Trắng)
    lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(status_panel, lv_color_hex(0x9E9E9E), 0);
    snprintf(buf_str, sizeof(buf_str), "Hiện tại: %s", ten_tram[currentLoc]);
    lv_label_set_text(status_label, buf_str);

    // Khung 2: Chỉ đường (Màu xanh dương)
    lv_obj_set_style_bg_color(dest_panel, lv_color_hex(0xE3F2FD), 0); // Xanh nhạt
    lv_obj_set_style_border_color(dest_panel, lv_color_hex(0x4285F4), 0);
    snprintf(buf_str, sizeof(buf_str), "=> %s", lay_huong_dan(currentLoc, destLoc));
    lv_label_set_text(dest_label, buf_str);

    lv_obj_clear_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_clear_flag(dest_panel, LV_OBJ_FLAG_HIDDEN);

    // Khung 1: Báo vị trí (Trắng)
    lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(status_panel, lv_color_hex(0x9E9E9E), 0);
    snprintf(buf_str, sizeof(buf_str), "Hiện tại: %s", ten_tram[currentLoc]);
    lv_label_set_text(status_label, buf_str);

    // Khung 2: Đang lướt chọn (Màu cam)
    lv_obj_set_style_bg_color(dest_panel, lv_color_hex(0xFFF3E0), 0); // Cam nhạt
    lv_obj_set_style_border_color(dest_panel, lv_color_hex(0xFF9800), 0);
    snprintf(buf_str, sizeof(buf_str), "Chọn: %s (Giữ chốt)", ten_tram[destLoc]);
    lv_label_set_text(dest_label, buf_str);

    lv_obj_clear_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);
  }

  // 5. VẼ MŨI TÊN CHỈ HƯỚNG BẢN THÂN
  int chieu_dai = 10, chieu_rong = 6, lom_vao = 3;
  int raw_pts[5][2] = {{0, -chieu_dai}, {chieu_rong, chieu_dai}, {0, chieu_dai - lom_vao}, {-chieu_rong, chieu_dai}, {0, -chieu_dai}};
  for (int i = 0; i < 5; i++)
  {
    line_points[i].x = (lv_coord_t)(current_smooth_x + raw_pts[i][0] * c - raw_pts[i][1] * s);
    line_points[i].y = (lv_coord_t)(current_smooth_y + raw_pts[i][0] * s + raw_pts[i][1] * c);
  }
  lv_line_set_points(kim_chi_nam, line_points, 5);
  lv_obj_move_foreground(kim_chi_nam);
}

// =======================================================
// SETUP & LOOP
// =======================================================
void setup()
{
  Serial.begin(115200);

  pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
  pinMode(PIN_BTN_PREV, INPUT_PULLUP);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

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
  initUiToGray();

  if (objects.pnl_map_2_group != NULL)
    lv_obj_clear_flag(objects.pnl_map_2_group, LV_OBJ_FLAG_HIDDEN);
  if (objects.pnl_text_2_group != NULL)
    lv_obj_clear_flag(objects.pnl_text_2_group, LV_OBJ_FLAG_HIDDEN);

  // ---------------------------------------------------
  // 1. KHUNG TRÊN: HIỂN THỊ VỊ TRÍ HIỆN TẠI
  // ---------------------------------------------------
  status_panel = lv_obj_create(lv_layer_top());
  lv_obj_set_size(status_panel, 220, 35);
  lv_obj_align(status_panel, LV_ALIGN_TOP_MID, 0, 5);
  lv_obj_clear_flag(status_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_width(status_panel, 2, 0);
  lv_obj_set_style_border_color(status_panel, lv_color_hex(0x9E9E9E), 0);
  lv_obj_set_style_radius(status_panel, 8, 0);
  lv_obj_set_style_pad_all(status_panel, 0, 0);

  status_label = lv_label_create(status_panel);
  lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(status_label, &ui_font_roboto, 0);
  lv_label_set_text(status_label, "Đang tìm vị trí...");

  // ---------------------------------------------------
  // 2. KHUNG DƯỚI: HIỂN THỊ CHỌN ĐIỂM / CHỈ ĐƯỜNG
  // ---------------------------------------------------
  dest_panel = lv_obj_create(lv_layer_top());
  lv_obj_set_size(dest_panel, 220, 35);
  lv_obj_align(dest_panel, LV_ALIGN_TOP_MID, 0, 45); // Đẩy lùi xuống 45px để nằm dưới khung 1
  lv_obj_clear_flag(dest_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(dest_panel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_width(dest_panel, 2, 0);
  lv_obj_set_style_border_color(dest_panel, lv_color_hex(0xFF9800), 0);
  lv_obj_set_style_radius(dest_panel, 8, 0);
  lv_obj_set_style_pad_all(dest_panel, 0, 0);
  lv_obj_add_flag(dest_panel, LV_OBJ_FLAG_HIDDEN); // Ẩn đi lúc vừa bật máy

  dest_label = lv_label_create(dest_panel);
  lv_obj_align(dest_label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(dest_label, &ui_font_roboto, 0);

  // Bật hiệu ứng chữ chạy cho khung dưới (đề phòng câu chỉ đường dài)
  lv_label_set_long_mode(dest_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(dest_label, 210);
  lv_label_set_text(dest_label, "Chọn nơi đến...");

  kim_chi_nam = lv_line_create(lv_layer_top());
  static lv_style_t style_line;
  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, 3);
  lv_style_set_line_color(&style_line, lv_color_hex(0xFF0000));
  lv_style_set_line_rounded(&style_line, true);
  lv_obj_add_style(kim_chi_nam, &style_line, 0);
  lv_obj_add_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);

  Wire.begin(26, 27);
  initHardwareQMC5883L();
  setupBLE();

  current_smooth_x = cac_tram[1].x + 15;
  current_smooth_y = cac_tram[1].y + 15;
}

void loop()
{
  unsigned long currentMillis = millis();

  xu_ly_nut_bam(); // Hàm đã được tách ra

  lv_tick_inc(currentMillis - lastTick);
  lastTick = currentMillis;
  lv_timer_handler();

  scanAndProcessBLE();
  xu_ly_dan_duong();

  delay(15);
}