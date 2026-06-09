#include "UI_Manager.h"
#include "ui.h" // Nhúng thư viện EEZ Studio

void switchMapAndText(int index)
{
    lv_obj_t *mapGroups[] = {objects.pnl_map_1_group, objects.pnl_map_2_group};
    lv_obj_t *textGroups[] = {objects.pnl_text_1_group, objects.pnl_text_2_group};

    for (int i = 0; i < totalMaps; i++)
    {
        if (i == index - 1)
        {
            lv_obj_clear_flag(mapGroups[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(textGroups[i], LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(mapGroups[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(textGroups[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void update_battery_display(int percent)
{
    if (percent <= 20)
        lv_obj_set_style_text_color(battery_label, lv_color_hex(0xFF0000), 0);
    else
        lv_obj_set_style_text_color(battery_label, lv_color_hex(0x000000), 0);

    if (percent >= 80)
        lv_label_set_text_fmt(battery_label, LV_SYMBOL_BATTERY_FULL " %d%%", percent);
    else if (percent >= 60)
        lv_label_set_text_fmt(battery_label, LV_SYMBOL_BATTERY_3 " %d%%", percent);
    else if (percent >= 40)
        lv_label_set_text_fmt(battery_label, LV_SYMBOL_BATTERY_2 " %d%%", percent);
    else if (percent >= 20)
        lv_label_set_text_fmt(battery_label, LV_SYMBOL_BATTERY_1 " %d%%", percent);
    else
        lv_label_set_text_fmt(battery_label, LV_SYMBOL_BATTERY_EMPTY " %d%%", percent);
}

// =================================================================
// 1. HÀM QUÉT RADAR: CHỈ TẮT CÁC CHẤM TRÊN BẢN ĐỒ (Gọi liên tục)
// =================================================================
void resetMapDotsToGray()
{
    lv_color_t colorGray = lv_color_hex(0x808080);

    // CHỈ TẮT CHẤM TRÊN BẢN ĐỒ
    lv_obj_set_style_bg_color(objects.dot_alpha, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_beta, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_gamma, colorGray, 0);

    lv_obj_set_style_bg_color(objects.dot_tuyensinh, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_wc, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_trienlam, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_it, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_thuvien, colorGray, 0);

    // TUYỆT ĐỐI KHÔNG ĐỤNG CHẠM GÌ TỚI CHẤM Ở PHẦN TEXT (LỊCH SỬ)
}

// =================================================================
// 2. HÀM KHỞI TẠO: TẮT SẠCH MỌI THỨ KHI VỪA BẬT MÁY (Chỉ gọi 1 lần)
// =================================================================
void initUiToGray()
{
    lv_color_t colorGray = lv_color_hex(0x808080);

    // A. Reset màu chữ
    lv_obj_set_style_text_color(objects.lbl_alpha, colorGray, 0);
    lv_obj_set_style_text_color(objects.lbl_beta, colorGray, 0);
    lv_obj_set_style_text_color(objects.lbl_gamma, colorGray, 0);

    lv_obj_set_style_text_color(objects.lbl_tuyensinh, colorGray, 0);
    lv_obj_set_style_text_color(objects.lbl_wc, colorGray, 0);
    lv_obj_set_style_text_color(objects.lbl_trienlam, colorGray, 0);
    lv_obj_set_style_text_color(objects.lbl_it, colorGray, 0);
    lv_obj_set_style_text_color(objects.lbl_thuvien, colorGray, 0);

    lv_obj_set_style_bg_color(objects.dot_lbl_alpha, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_lbl_beta, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_lbl_gamma, colorGray, 0);

    lv_obj_set_style_bg_color(objects.dot_lbl_tuyensinh, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_lbl_wc, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_lbl_trienlam, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_lbl_it, colorGray, 0);
    lv_obj_set_style_bg_color(objects.dot_lbl_thuvien, colorGray, 0);

    for (int i = 0; i < 15; i++)
        isCheckedIn[i] = false;

    resetMapDotsToGray();
}

void checkInHistory(int locationID)
{
    if (!isCheckedIn[locationID])
    {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(100);
        digitalWrite(PIN_BUZZER, LOW);
        delay(50);
        digitalWrite(PIN_BUZZER, HIGH);
        delay(100);
        digitalWrite(PIN_BUZZER, LOW);

        isCheckedIn[locationID] = true;
        lv_color_t dotGreen = lv_color_hex(0x00FF00);
        lv_color_t textRed = lv_color_hex(0xFF0000);

        switch (locationID)
        {
        case 1:
            lv_obj_set_style_text_color(objects.lbl_alpha, textRed, 0);
            lv_obj_set_style_bg_color(objects.dot_lbl_alpha, dotGreen, 0);
            break;
        case 2:
            lv_obj_set_style_text_color(objects.lbl_beta, textRed, 0);
            lv_obj_set_style_bg_color(objects.dot_lbl_beta, dotGreen, 0);
            break;
        case 3:
            lv_obj_set_style_text_color(objects.lbl_gamma, textRed, 0);
            lv_obj_set_style_bg_color(objects.dot_lbl_gamma, dotGreen, 0);
            break;

        case 7:
            lv_obj_set_style_text_color(objects.lbl_tuyensinh, textRed, 0);
            lv_obj_set_style_bg_color(objects.dot_lbl_tuyensinh, dotGreen, 0);
            break;
        case 8:
            lv_obj_set_style_text_color(objects.lbl_wc, textRed, 0);
            lv_obj_set_style_bg_color(objects.dot_lbl_wc, dotGreen, 0);
            break;
        case 9:
            lv_obj_set_style_text_color(objects.lbl_trienlam, textRed, 0);
            lv_obj_set_style_bg_color(objects.dot_lbl_trienlam, dotGreen, 0);
            break;
        case 10:
            lv_obj_set_style_text_color(objects.lbl_it, textRed, 0);
            lv_obj_set_style_bg_color(objects.dot_lbl_it, dotGreen, 0);
            break;
        case 11:
            lv_obj_set_style_text_color(objects.lbl_thuvien, textRed, 0);
            lv_obj_set_style_bg_color(objects.dot_lbl_thuvien, dotGreen, 0);
            break;
        }
    }
}

void turnOnMapDot(int locationID)
{
    lv_color_t dotGreen = lv_color_hex(0x00FF00);
    switch (locationID)
    {
    case 1:
        lv_obj_set_style_bg_color(objects.dot_alpha, dotGreen, 0);
        break;
    case 2:
        lv_obj_set_style_bg_color(objects.dot_beta, dotGreen, 0);
        break;
    case 3:
        lv_obj_set_style_bg_color(objects.dot_gamma, dotGreen, 0);
        break;

    case 7:
        lv_obj_set_style_bg_color(objects.dot_tuyensinh, dotGreen, 0);
        break;
    case 8:
        lv_obj_set_style_bg_color(objects.dot_wc, dotGreen, 0);
        break;
    case 9:
        lv_obj_set_style_bg_color(objects.dot_trienlam, dotGreen, 0);
        break;
    case 10:
        lv_obj_set_style_bg_color(objects.dot_it, dotGreen, 0);
        break;
    case 11:
        lv_obj_set_style_bg_color(objects.dot_thuvien, dotGreen, 0);
        break;
    }
}