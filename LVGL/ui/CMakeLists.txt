SET(SOURCES screens/ui_ScreenStart.c
    screens/ui_ScreenLogin.c
    screens/ui_ScreenMenu.c
    screens/ui_ScreenMonitor.c
    screens/ui_ScreenSetTime.c
    screens/ui_ScreenSureSetTime.c
    screens/ui_ScreenEnroll.c
    screens/ui_ScreenLightCtrl.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_protect111_png.c
    images/ui_img_lock_png.c
    images/ui_img_time_png.c
    images/ui_img_search_house_png.c
    images/ui_img_light_png.c
    images/ui_img_temperature_png.c
    images/ui_img_humidity_png.c
    images/ui_img_fail_png.c
    images/ui_img_mq2_png.c
    images/ui_img_fire_png.c
    images/ui_img_success_png.c)

add_library(ui ${SOURCES})
