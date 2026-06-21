#include "ui/UI_Manager.h"
#include "ui/ui.h"
#include "ui/screens.h"
#include "Globals.h"

void switchMapAndText(int index) {}
void update_battery_display(int percent) {}

void resetMapDotsToGray()
{
    lv_color_t colorGray = lv_color_hex(0x808080);
    if (objects.dot_tuyensinh != NULL)
        lv_obj_set_style_bg_color(objects.dot_tuyensinh, colorGray, 0);
    if (objects.dot_it != NULL)
        lv_obj_set_style_bg_color(objects.dot_it, colorGray, 0);
    if (objects.dot_thuvien != NULL)
        lv_obj_set_style_bg_color(objects.dot_thuvien, colorGray, 0);
    if (objects.dot_wc != NULL)
        lv_obj_set_style_bg_color(objects.dot_wc, colorGray, 0);
}

void initUiToGray()
{
    lv_color_t colorGray = lv_color_hex(0x808080);

    if (objects.lbl_tuyensinh != NULL)
        lv_obj_set_style_text_color(objects.lbl_tuyensinh, colorGray, 0);
    if (objects.lbl_it != NULL)
        lv_obj_set_style_text_color(objects.lbl_it, colorGray, 0);
    if (objects.lbl_thuvien != NULL)
        lv_obj_set_style_text_color(objects.lbl_thuvien, colorGray, 0);
    if (objects.lbl_wc != NULL)
        lv_obj_set_style_text_color(objects.lbl_wc, colorGray, 0);

    if (objects.dot_lbl_tuyensinh != NULL)
        lv_obj_set_style_bg_color(objects.dot_lbl_tuyensinh, colorGray, 0);
    if (objects.dot_lbl_it != NULL)
        lv_obj_set_style_bg_color(objects.dot_lbl_it, colorGray, 0);
    if (objects.dot_lbl_thuvien != NULL)
        lv_obj_set_style_bg_color(objects.dot_lbl_thuvien, colorGray, 0);
    if (objects.dot_lbl_wc != NULL)
        lv_obj_set_style_bg_color(objects.dot_lbl_wc, colorGray, 0);

    for (int i = 0; i < 15; i++)
    {
        isCheckedIn[i] = false;
    }
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
            if (objects.lbl_tuyensinh != NULL)
                lv_obj_set_style_text_color(objects.lbl_tuyensinh, textRed, 0);
            if (objects.dot_lbl_tuyensinh != NULL)
                lv_obj_set_style_bg_color(objects.dot_lbl_tuyensinh, dotGreen, 0);
            break;
        case 2:
            if (objects.lbl_it != NULL)
                lv_obj_set_style_text_color(objects.lbl_it, textRed, 0);
            if (objects.dot_lbl_it != NULL)
                lv_obj_set_style_bg_color(objects.dot_lbl_it, dotGreen, 0);
            break;
        case 3:
            if (objects.lbl_thuvien != NULL)
                lv_obj_set_style_text_color(objects.lbl_thuvien, textRed, 0);
            if (objects.dot_lbl_thuvien != NULL)
                lv_obj_set_style_bg_color(objects.dot_lbl_thuvien, dotGreen, 0);
            break;
        case 4:
            if (objects.lbl_wc != NULL)
                lv_obj_set_style_text_color(objects.lbl_wc, textRed, 0);
            if (objects.dot_lbl_wc != NULL)
                lv_obj_set_style_bg_color(objects.dot_lbl_wc, dotGreen, 0);
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
        if (objects.dot_tuyensinh != NULL)
            lv_obj_set_style_bg_color(objects.dot_tuyensinh, dotGreen, 0);
        break;
    case 2:
        if (objects.dot_it != NULL)
            lv_obj_set_style_bg_color(objects.dot_it, dotGreen, 0);
        break;
    case 3:
        if (objects.dot_thuvien != NULL)
            lv_obj_set_style_bg_color(objects.dot_thuvien, dotGreen, 0);
        break;
    case 4:
        if (objects.dot_wc != NULL)
            lv_obj_set_style_bg_color(objects.dot_wc, dotGreen, 0);
        break;
    }
}