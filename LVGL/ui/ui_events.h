// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.0
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

void ChangePassword(lv_event_t * e);
void ChangeAdmin(lv_event_t * e);
void eLoginToMenu(lv_event_t * e);
void HoursInit(lv_event_t * e);
void MinuteInit(lv_event_t * e);
void SecondInit(lv_event_t * e);
void SecondSelect(lv_event_t * e);
void HoursSlect(lv_event_t * e);
void MinuteSelect(lv_event_t * e);
void RtcTimeSet(lv_event_t * e);
void ChangeTime(lv_event_t * e);
void enChangeAdmin(lv_event_t * e);
void enChangePassword(lv_event_t * e);
void enChangeRePassword(lv_event_t * e);
void fEnrollAdmin(lv_event_t * e);
void Led1Change(lv_event_t * e);
void Led2Change(lv_event_t * e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
