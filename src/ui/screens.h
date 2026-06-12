#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    _SCREEN_ID_LAST = 1
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *pnl_map;
    lv_obj_t *pnl_map_2_group;
    lv_obj_t *img_map_2_bg;
    lv_obj_t *dot_thuvien;
    lv_obj_t *dot_cuachinh;
    lv_obj_t *dot_it;
    lv_obj_t *dot_trienlam;
    lv_obj_t *dot_wc;
    lv_obj_t *dot_tuyensinh;
    lv_obj_t *pnl_map_1_group;
    lv_obj_t *img_map_1_bg;
    lv_obj_t *dot_cantin;
    lv_obj_t *dot_nhavo;
    lv_obj_t *dot_gamma;
    lv_obj_t *dot_beta;
    lv_obj_t *dot_nhaxe;
    lv_obj_t *dot_alpha;
    lv_obj_t *pnl_text;
    lv_obj_t *pnl_text_2_group;
    lv_obj_t *lbl_tuyensinh;
    lv_obj_t *dot_lbl_tuyensinh;
    lv_obj_t *lbl_wc;
    lv_obj_t *dot_lbl_wc;
    lv_obj_t *lbl_trienlam;
    lv_obj_t *dot_lbl_trienlam;
    lv_obj_t *lbl_it;
    lv_obj_t *dot_lbl_it;
    lv_obj_t *lbl_thuvien;
    lv_obj_t *dot_lbl_thuvien;
    lv_obj_t *lbl_cuachinh;
    lv_obj_t *dot_lbl_cuachinh;
    lv_obj_t *pnl_text_1_group;
    lv_obj_t *lbl_nhavo;
    lv_obj_t *dot_lbl_nhavo;
    lv_obj_t *lbl_cantin;
    lv_obj_t *dot_lbl_cantin;
    lv_obj_t *lbl_nhaxe;
    lv_obj_t *dot_lbl_nhaxe;
    lv_obj_t *lbl_gamma;
    lv_obj_t *dot_lbl_gamma;
    lv_obj_t *lbl_beta;
    lv_obj_t *dot_lbl_beta;
    lv_obj_t *lbl_alpha;
    lv_obj_t *dot_lbl_alpha;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/