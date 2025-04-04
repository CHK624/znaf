// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.0
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

#include "ui_helpers.h"
#include "ui_events.h"

// SCREEN: ui_ScreenStart
void ui_ScreenStart_screen_init(void);
void ui_event_ScreenStart(lv_event_t * e);
extern lv_obj_t * ui_ScreenStart;
extern lv_obj_t * ui_title;
void ui_event_Sliderstart(lv_event_t * e);
extern lv_obj_t * ui_Sliderstart;
extern lv_obj_t * ui_LabelTime1;
extern lv_obj_t * ui_Image1;
// SCREEN: ui_ScreenLogin
void ui_ScreenLogin_screen_init(void);
void ui_event_ScreenLogin(lv_event_t * e);
extern lv_obj_t * ui_ScreenLogin;
extern lv_obj_t * ui_Keyboard1;
void ui_event_TextAreaPassword(lv_event_t * e);
extern lv_obj_t * ui_TextAreaPassword;
void ui_event_TextAreaAdmin(lv_event_t * e);
extern lv_obj_t * ui_TextAreaAdmin;
void ui_event_ButtonLogin(lv_event_t * e);
extern lv_obj_t * ui_ButtonLogin;
void ui_event_LabelLogin(lv_event_t * e);
extern lv_obj_t * ui_LabelLogin;
void ui_event_ButtonEnroll(lv_event_t * e);
extern lv_obj_t * ui_ButtonEnroll;
extern lv_obj_t * ui_LabelEnroll;
// SCREEN: ui_ScreenMenu
void ui_ScreenMenu_screen_init(void);
extern lv_obj_t * ui_ScreenMenu;
void ui_event_ImgButtonLock(lv_event_t * e);
extern lv_obj_t * ui_ImgButtonLock;
extern lv_obj_t * ui_LabelTime2;
void ui_event_ImgButtonTimeSet(lv_event_t * e);
extern lv_obj_t * ui_ImgButtonTimeSet;
void ui_event_ImgButtonMonitor(lv_event_t * e);
extern lv_obj_t * ui_ImgButtonMonitor;
void ui_event_MenuToLight(lv_event_t * e);
extern lv_obj_t * ui_MenuToLight;
// SCREEN: ui_ScreenMonitor
void ui_ScreenMonitor_screen_init(void);
extern lv_obj_t * ui_ScreenMonitor;
extern lv_obj_t * ui_LabelTime3;
extern lv_obj_t * ui_temperature;
extern lv_obj_t * ui_humiture;
extern lv_obj_t * ui_BarTemperature;
extern lv_obj_t * ui_LableTemperature;
extern lv_obj_t * ui_BarTemHumiture;
extern lv_obj_t * ui_LableHumiture;
void ui_event_ImgButton2(lv_event_t * e);
extern lv_obj_t * ui_ImgButton2;
extern lv_obj_t * ui_temperature2;
extern lv_obj_t * ui_temperature3;
extern lv_obj_t * ui_ButtonMq2Status;
extern lv_obj_t * ui_LabelMq2Status;
extern lv_obj_t * ui_ButtonFireStatus;
extern lv_obj_t * ui_LabelFireStatus;
// SCREEN: ui_ScreenSetTime
void ui_ScreenSetTime_screen_init(void);
void ui_event_ScreenSetTime(lv_event_t * e);
extern lv_obj_t * ui_ScreenSetTime;
void ui_event_ImgButtonSetTimeSes(lv_event_t * e);
extern lv_obj_t * ui_ImgButtonSetTimeSes;
void ui_event_RollerSecond(lv_event_t * e);
extern lv_obj_t * ui_RollerSecond;
void ui_event_RollerHours(lv_event_t * e);
extern lv_obj_t * ui_RollerHours;
void ui_event_RollerMinute(lv_event_t * e);
extern lv_obj_t * ui_RollerMinute;
extern lv_obj_t * ui_Labelhours;
extern lv_obj_t * ui_LabelTimeSet;
extern lv_obj_t * ui_Labelminute;
extern lv_obj_t * ui_Labelsecond;
void ui_event_ImgButton3(lv_event_t * e);
extern lv_obj_t * ui_ImgButton3;
// SCREEN: ui_ScreenSureSetTime
void ui_ScreenSureSetTime_screen_init(void);
extern lv_obj_t * ui_ScreenSureSetTime;
extern lv_obj_t * ui_Label1;
void ui_event_Button2(lv_event_t * e);
extern lv_obj_t * ui_Button2;
extern lv_obj_t * ui_Label2;
void ui_event_Button3(lv_event_t * e);
extern lv_obj_t * ui_Button3;
void ui_event_Label3(lv_event_t * e);
extern lv_obj_t * ui_Label3;
// SCREEN: ui_ScreenEnroll
void ui_ScreenEnroll_screen_init(void);
extern lv_obj_t * ui_ScreenEnroll;
void ui_event_ImgButton1(lv_event_t * e);
extern lv_obj_t * ui_ImgButton1;
extern lv_obj_t * ui_Keyboard2;
void ui_event_TextAreaEnrllAdmin(lv_event_t * e);
extern lv_obj_t * ui_TextAreaEnrllAdmin;
void ui_event_TextAreaEnrllpswd(lv_event_t * e);
extern lv_obj_t * ui_TextAreaEnrllpswd;
void ui_event_TextAreaEnrllrepswd(lv_event_t * e);
extern lv_obj_t * ui_TextAreaEnrllrepswd;
void ui_event_imgButtonEnrollSure(lv_event_t * e);
extern lv_obj_t * ui_imgButtonEnrollSure;
// SCREEN: ui_ScreenLightCtrl
void ui_ScreenLightCtrl_screen_init(void);
extern lv_obj_t * ui_ScreenLightCtrl;
extern lv_obj_t * ui_Image2;
void ui_event_Button6(lv_event_t * e);
extern lv_obj_t * ui_Button6;
extern lv_obj_t * ui_Label4;
void ui_event_Button4(lv_event_t * e);
extern lv_obj_t * ui_Button4;
extern lv_obj_t * ui_Label6;
void ui_event_ImgButtonExitTOLight(lv_event_t * e);
extern lv_obj_t * ui_ImgButtonExitTOLight;
extern lv_obj_t * ui____initial_actions0;

LV_IMG_DECLARE(ui_img_protect111_png);    // assets/protect111.png
LV_IMG_DECLARE(ui_img_lock_png);    // assets/lock.png
LV_IMG_DECLARE(ui_img_time_png);    // assets/time.png
LV_IMG_DECLARE(ui_img_search_house_png);    // assets/search house.png
LV_IMG_DECLARE(ui_img_light_png);    // assets/light.png
LV_IMG_DECLARE(ui_img_temperature_png);    // assets/temperature.png
LV_IMG_DECLARE(ui_img_humidity_png);    // assets/humidity.png
LV_IMG_DECLARE(ui_img_fail_png);    // assets/fail.png
LV_IMG_DECLARE(ui_img_mq2_png);    // assets/MQ2.png
LV_IMG_DECLARE(ui_img_fire_png);    // assets/fire.png
LV_IMG_DECLARE(ui_img_success_png);    // assets/success.png




void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
