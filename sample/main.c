//#include "../lvgl/examples/lv_examples.h"
#include "../lvgl/demos/lv_demos.h"
#include "../lvgl/demos/keypad_encoder/lv_demo_keypad_encoder.h"
#include "../lv_port_disp.h"
#include "../lv_port_indev.h"

#include <pspkernel.h>

PSP_MODULE_INFO("LVGL Sample", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

// forward decl
int tick_thread_proc(SceSize argc, void* argp);

int main(int argc, char** argv)
{
    SceUID tick_thread = sceKernelCreateThread("Tick Thread", tick_thread_proc, 0x10, 0x10000, 0, NULL);
    sceKernelStartThread(tick_thread, 0, 0);
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    //lv_example_get_started_3();
    lv_demo_music();
    //lv_demo_keypad_encoder();
    while (true) {
        lv_timer_handler();
        sceKernelDelayThread(5000);
    }
}


int tick_thread_proc(SceSize argc, void* argp)
{
    while (true) {
        lv_tick_inc(5);
        sceKernelDelayThread(5000);
    }
}
