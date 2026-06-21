#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <lvgl.h>

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
// BỘ NÃO ĐỊNH VỊ: 4 con số Offset vàng ông đã đo được!
// [0: Không dùng, 1: Cửa Số 1, 2: IT, 3: Thư Viện, 4: WC]
float station_offsets[5] = {0.0, -75.2, -10.7, -29.3, -54.0};

LV_FONT_DECLARE(ui_font_roboto)

bool isCheckedIn[15] = {false};
int currentMapIndex = 2;
int totalMaps = 1;
bool systemOn = true;

extern int detectedMajor;

int targetLocID = 2;
bool isNavigating = false;

struct ToaDoMain
{
  int x;
  int y;
};
ToaDoMain cac_tram[] = {
    {0, 0},
    {34, 58},   // Trạm 1: Cửa Số 1
    {10, 112},  // Trạm 2: IT
    {142, 161}, // Trạm 3: Thư viện
    {220, 61}   // Trạm 4: WC
};

const char *ten_tram[] = {"Không xác định", "Cửa Số 1", "Phòng IT", "Thư Viện", "Nhà WC"};

// =======================================================
// ĐỐI TƯỢNG ĐỒ HỌA
// =======================================================
lv_obj_t *kim_chi_nam;
lv_obj_t *status_panel;
lv_obj_t *dest_panel;
lv_obj_t *non_chi_huong;
lv_obj_t *diem_hien_tai;
lv_obj_t *dest_label;
lv_obj_t *status_label;

static lv_point_t line_points[5];
static lv_point_t beam_points[4];

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
  int offset_x = 8, offset_y = 8 + 59;

  // Bộ lọc tĩnh chống khựng/reset chữ chạy của LVGL
  static char last_status_text[128] = "";
  static char last_dest_text[128] = "";

  // Cập nhật chấm trên bản đồ khi đổi trạm
  static int last_ui_loc = -1;
  if (currentLoc > 0 && currentLoc != last_ui_loc)
  {
    resetMapDotsToGray();
    turnOnMapDot(currentLoc);
    checkInHistory(currentLoc);
    last_ui_loc = currentLoc;
  }

  // Lọc nhiễu la bàn: Nếu cảm biến báo lỗi (-1), giữ nguyên góc cũ
  float raw_compass = getCompassHeading();
  if (raw_compass < 0)
    return;

  // 1. QUẢN LÝ TRẠNG THÁI "ĐÃ ĐẾN NƠI"
  static bool dang_giu_man_hinh_den = false;
  if (isNavigating && currentLoc > 0 && destLoc > 0 && currentLoc == destLoc)
  {
    dang_giu_man_hinh_den = true;
    isNavigating = false;
  }
  else if (currentLoc != destLoc)
  {
    dang_giu_man_hinh_den = false;
  }

  // 2. Lấy hướng la bàn thô và bù trừ bằng Offset riêng của từng trạm
  float rad_compass = raw_compass * M_PI / 180.0;
  float offset = station_offsets[currentLoc] * M_PI / 180.0;
  float rad_compass_corrected = rad_compass + offset;

  // 3. Tính tọa độ chấm xanh mượt mà
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

  // 4. Tính hướng đến đích (Góc tĩnh trên bản đồ)
  float rad_dest = 0;
  if (currentLoc > 0 && destLoc > 0)
  {
    float dx = cac_tram[destLoc].x - cac_tram[currentLoc].x;
    float dy = cac_tram[destLoc].y - cac_tram[currentLoc].y;
    rad_dest = atan2(dx, -dy);
  }

  // Góc bẻ lái: Sự chênh lệch giữa Hướng Đích và Hướng Mặt người dùng
  float rad_steering = rad_dest - rad_compass_corrected;
  while (rad_steering > M_PI)
    rad_steering -= 2 * M_PI;
  while (rad_steering < -M_PI)
    rad_steering += 2 * M_PI;

  // =================================================================
  // ĐOẠN LOG ĐỂ THEO DÕI QUA SERIAL MONITOR
  // =================================================================
  static unsigned long last_log_time = 0;
  if (millis() - last_log_time > 1500)
  {
    float goc_map_do = rad_dest * 180.0 / M_PI;
    if (goc_map_do < 0)
      goc_map_do += 360.0;

    // Serial.println("--------------------------------------------------");
    // Serial.printf("📍 DANG O TRAM: %d (%s)  =>  DI DEN: %d (%s)\n", currentLoc, ten_tram[currentLoc], destLoc, ten_tram[destLoc]);
    // Serial.printf("   + Goc tren ban do (goc_di)   : %.1f do\n", goc_map_do);
    // Serial.printf("   + Goc la ban thuc te (Raw)   : %.1f do\n", raw_compass);
    // Serial.printf("   + Offset hien tai dang dung  : %.1f do\n", station_offsets[currentLoc]);
    // Serial.println("--------------------------------------------------");

    last_log_time = millis();
  }

  // =======================================================
  // 5. QUẢN LÝ GIAO DIỆN UI (ĐÃ FIX TRƯỢT CHỮ MARQUEE)
  // =======================================================
  if (currentLoc == 0)
  {
    lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
    if (strcmp(last_status_text, "Đang tìm vị trí...") != 0)
    {
      lv_label_set_text(status_label, "Đang tìm vị trí...");
      strcpy(last_status_text, "Đang tìm vị trí...");
    }
    lv_obj_add_flag(dest_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(diem_hien_tai, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(non_chi_huong, LV_OBJ_FLAG_HIDDEN);
    last_dest_text[0] = '\0'; // Xóa cache chữ dưới
    return;
  }
  else if (dang_giu_man_hinh_den)
  {
    lv_obj_clear_flag(dest_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(status_panel, lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_border_color(status_panel, lv_color_hex(0x2E7D32), 0);
    if (strcmp(last_status_text, "Đã đến nơi!") != 0)
    {
      lv_label_set_text(status_label, "Đã đến nơi!");
      strcpy(last_status_text, "Đã đến nơi!");
    }

    lv_obj_set_style_bg_color(dest_panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(dest_panel, lv_color_hex(0x4CAF50), 0);
    snprintf(buf_str, sizeof(buf_str), "Mục tiêu: %s", ten_tram[destLoc]);
    if (strcmp(last_dest_text, buf_str) != 0)
    {
      lv_label_set_text(dest_label, buf_str);
      strcpy(last_dest_text, buf_str);
    }

    lv_obj_add_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(diem_hien_tai, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(non_chi_huong, LV_OBJ_FLAG_HIDDEN);
  }
  else if (isNavigating)
  {
    lv_obj_clear_flag(dest_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(status_panel, lv_color_hex(0x9E9E9E), 0);

    char status_buf[128];
    snprintf(status_buf, sizeof(status_buf), "Bạn Đang Ở: %s", ten_tram[currentLoc]);
    if (strcmp(last_status_text, status_buf) != 0)
    {
      lv_label_set_text(status_label, status_buf);
      strcpy(last_status_text, status_buf);
    }

    lv_obj_set_style_bg_color(dest_panel, lv_color_hex(0xE3F2FD), 0);
    lv_obj_set_style_border_color(dest_panel, lv_color_hex(0x4285F4), 0);
    snprintf(buf_str, sizeof(buf_str), "=> %s", lay_huong_dan(currentLoc, destLoc));
    if (strcmp(last_dest_text, buf_str) != 0)
    {
      lv_label_set_text(dest_label, buf_str);
      strcpy(last_dest_text, buf_str);
    }

    lv_obj_clear_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(diem_hien_tai, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(non_chi_huong, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    // TRẠNG THÁI CHỜ CHỌN ĐỊA ĐIỂM (ĐẦU TIÊN CÓ TÍN HIỆU)
    lv_obj_clear_flag(dest_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_color(status_panel, lv_color_hex(0x9E9E9E), 0);

    char status_buf[128];
    snprintf(status_buf, sizeof(status_buf), "Bạn Đang Ở: %s", ten_tram[currentLoc]);
    if (strcmp(last_status_text, status_buf) != 0)
    {
      lv_label_set_text(status_label, status_buf);
      strcpy(last_status_text, status_buf);
    }

    lv_obj_set_style_bg_color(dest_panel, lv_color_hex(0xFFF3E0), 0);
    lv_obj_set_style_border_color(dest_panel, lv_color_hex(0xFF9800), 0);

    // FIX THEO YÊU CẦU: Hiện chuỗi chữ chạy dài, có hướng dẫn "Chọn điểm bạn muốn đến" rõ ràng
    snprintf(buf_str, sizeof(buf_str), "Bạn muốn đến: %s", ten_tram[destLoc]);
    if (strcmp(last_dest_text, buf_str) != 0)
    {
      lv_label_set_text(dest_label, buf_str);
      strcpy(last_dest_text, buf_str);
    }

    lv_obj_add_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(diem_hien_tai, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(non_chi_huong, LV_OBJ_FLAG_HIDDEN);
  }

  // =======================================================
  // 6. ĐỒNG BỘ TOÀN BỘ TỌA ĐỘ VÀ LAYER ĐỒ HỌA
  // =======================================================

  // A. VẼ CHẤM XANH
  lv_obj_set_pos(diem_hien_tai, (lv_coord_t)(current_smooth_x - 7), (lv_coord_t)(current_smooth_y - 7));

  // B. VẼ TIA LASER ĐỊNH VỊ (Xoay theo hướng mặt người dùng)
  lv_obj_set_pos(non_chi_huong, 0, 0);
  float laser_length = 30.0;
  beam_points[0].x = (lv_coord_t)current_smooth_x;
  beam_points[0].y = (lv_coord_t)current_smooth_y;
  beam_points[1].x = (lv_coord_t)(current_smooth_x + laser_length * sin(rad_compass_corrected));
  beam_points[1].y = (lv_coord_t)(current_smooth_y - laser_length * cos(rad_compass_corrected));
  lv_line_set_points(non_chi_huong, beam_points, 2);

  // C. VẼ MŨI TÊN DẪN ĐƯỜNG
  lv_obj_set_pos(kim_chi_nam, 0, 0);

  if (abs(rad_steering) < 0.35)
    lv_obj_set_style_line_color(kim_chi_nam, lv_color_hex(0x00FF00), 0);
  else
    lv_obj_set_style_line_color(kim_chi_nam, lv_color_hex(0xFF0000), 0);

  float c_rot = cos(rad_dest);
  float s_rot = sin(rad_dest);

  int c_dai = 16, c_rong = 9, lom = 4;
  int pts[5][2] = {{0, -c_dai}, {c_rong, lom}, {0, 0}, {-c_rong, lom}, {0, -c_dai}};
  for (int i = 0; i < 5; i++)
  {
    line_points[i].x = (lv_coord_t)(current_smooth_x + (pts[i][0] * c_rot - pts[i][1] * s_rot));
    line_points[i].y = (lv_coord_t)(current_smooth_y + (pts[i][0] * s_rot + pts[i][1] * c_rot));
  }
  lv_line_set_points(kim_chi_nam, line_points, 5);

  lv_obj_move_foreground(diem_hien_tai);
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
  tft.init(240, 320);
  tft.invertDisplay(false);
  tft.setRotation(2);
  tft.fillScreen(0x0000);
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

  status_panel = lv_obj_create(lv_layer_top());
  lv_obj_set_size(status_panel, 220, 30);
  lv_obj_align(status_panel, LV_ALIGN_TOP_MID, 0, 6);
  lv_obj_clear_flag(status_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_bg_opa(status_panel, 240, 0);
  lv_obj_set_style_radius(status_panel, 8, 0);
  lv_obj_set_style_pad_all(status_panel, 0, 0);
  status_label = lv_label_create(status_panel);
  lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(status_label, &ui_font_roboto, 0);
  lv_label_set_long_mode(status_label, LV_LABEL_LONG_DOT);
  lv_obj_set_width(status_label, 210);
  lv_label_set_text(status_label, "Đang tìm vị trí...");

  dest_panel = lv_obj_create(lv_layer_top());
  lv_obj_set_size(dest_panel, 220, 30);
  lv_obj_align(dest_panel, LV_ALIGN_TOP_MID, 0, 40);
  lv_obj_clear_flag(dest_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(dest_panel, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_bg_opa(dest_panel, 240, 0);
  lv_obj_set_style_radius(dest_panel, 8, 0);
  lv_obj_set_style_pad_all(dest_panel, 0, 0);
  lv_obj_add_flag(dest_panel, LV_OBJ_FLAG_HIDDEN);
  dest_label = lv_label_create(dest_panel);
  lv_obj_align(dest_label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(dest_label, &ui_font_roboto, 0);

  // SỬA TẠI ĐÂY: Chuyển hẳn sang chế độ LONG_SCROLL_CIRCULAR để chữ chạy trượt mượt mà liên tục
  lv_label_set_long_mode(dest_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(dest_label, 210);

  kim_chi_nam = lv_line_create(lv_layer_top());
  static lv_style_t s_line;
  lv_style_init(&s_line);
  lv_style_set_line_width(&s_line, 3);
  lv_style_set_line_color(&s_line, lv_color_hex(0xFF0000));
  lv_obj_add_style(kim_chi_nam, &s_line, 0);
  lv_obj_add_flag(kim_chi_nam, LV_OBJ_FLAG_HIDDEN);

  diem_hien_tai = lv_obj_create(lv_layer_top());
  lv_obj_set_size(diem_hien_tai, 14, 14);
  lv_obj_set_style_radius(diem_hien_tai, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(diem_hien_tai, lv_color_hex(0x4285F4), 0);
  lv_obj_set_style_border_width(diem_hien_tai, 2, 0);
  lv_obj_set_style_border_color(diem_hien_tai, lv_color_hex(0xFFFFFF), 0);
  lv_obj_clear_flag(diem_hien_tai, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(diem_hien_tai, LV_OBJ_FLAG_HIDDEN);

  non_chi_huong = lv_line_create(lv_layer_top());
  static lv_style_t style_beam;
  lv_style_init(&style_beam);
  lv_style_set_line_width(&style_beam, 3);
  lv_style_set_line_color(&style_beam, lv_color_hex(0x81C784));
  lv_style_set_line_rounded(&style_beam, true);
  lv_obj_add_style(non_chi_huong, &style_beam, 0);
  lv_obj_add_flag(non_chi_huong, LV_OBJ_FLAG_HIDDEN);

  // Khởi tạo phần cứng la bàn HMC5883L mới
  Wire.begin(26, 27);
  Wire.setClock(100000);
  Wire.setTimeout(1000);
  initHardwareQMC5883L();
  setupBLE();
  current_smooth_x = -1;
  current_smooth_y = -1;
}

void loop()
{
  unsigned long now = millis();
  xu_ly_nut_bam();
  lv_tick_inc(now - lastTick);
  lastTick = now;
  lv_timer_handler();
  scanAndProcessBLE();
  xu_ly_dan_duong();
  delay(15);
}