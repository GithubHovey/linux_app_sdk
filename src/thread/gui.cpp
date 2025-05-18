#include "robot_thread.h"


#include "lvgl/lvgl.h"
#include "lv_drivers/display/drm.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"

#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"
lv_ui guider_ui;

// int QUIT_FLAG;
float SCALE;
int WIFI_ENABLE;
int MUSIC_ENABLE;
extern volatile int QUIT_FLAG;
//static char* TSDEV="/dev/input/event0";
void gui_thread() {
    // pthread_setname_np(pthread_self(), "skg_gui");
    std::cout << "gui thread start..." << std::endl;
    int ret;

    /*GUI-Guider app Init*/
    custom_init();

    /* Get System Info */
    ret = luckfox_get_system_info();
    if(!ret)
    {
        /* Get Chip WIIF Info */
        // luckfox_get_wifi_enable_info();

        /* Check Music Dir */
        // luckfox_check_music_enable_info();
    }

    /* Get DRM Info */
    luckfox_get_drm_info(); // Set SCALE
    int disp_width = LV_HOR_RES_MAX * SCALE;
    int disp_height = LV_VER_RES_MAX * SCALE;
    const int disp_buf_size = disp_width * disp_height / 2;

    /*LittlevGL init*/
    lv_init();

    /*Linux FrameBuffer or DRM device init*/
    fbdev_init();
    // drm_init();
    
    /*Two buffer for LittlevGL to draw the screen's content*/
    lv_color_t buf0[disp_buf_size];
    lv_color_t buf1[disp_buf_size];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf0, buf1, disp_buf_size);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush; 
    // disp_drv.flush_cb   = drm_flush;
    disp_drv.hor_res    = disp_width;
    disp_drv.ver_res    = disp_height;
    lv_disp_drv_register(&disp_drv);

    /*Initialize and register a Input driver*/
    //luckfox_get_input_device_info(&TSDEV);
    //evdev_set_file(TSDEV);
    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);

    /*Create a GUI-Guider app */
    setup_ui(&guider_ui);
    events_init(&guider_ui);
   
    /*Handle LitlevGL tasks (tickless mode) */
    while(!QUIT_FLAG) {
        lv_timer_handler();
        usleep(5000);
    }

    /*Backend handler release */ 
    // if( WIFI_ENABLE == 1 )
    //     wifi_backend_release();
    // main_backend_release();

    system("cat /dev/zero > /dev/fb0");
    // return 0;
}