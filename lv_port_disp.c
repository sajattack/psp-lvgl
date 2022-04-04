/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "lvgl/lvgl.h"
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspge.h>
#include <vram.h>
#include <pspkerneltypes.h>
#include <pspdmac.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/
static unsigned int __attribute__((aligned(16))) DisplayList[262144];
static void* draw_buf;
static void* disp_buf;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-----------------------------
     * Create buffers for drawing
     *----------------------------*/

    draw_buf = valloc(PSP_BUF_WIDTH*PSP_VERT_RES*4);
    disp_buf = valloc(PSP_BUF_WIDTH*PSP_VERT_RES*4);


    /*-------------------------
     * Initialize your display
     * -----------------------*/

    disp_init();

    static lv_disp_draw_buf_t draw_buf_dsc_3;
    lv_disp_draw_buf_init(&draw_buf_dsc_3, draw_buf, disp_buf,
                          PSP_BUF_WIDTH * PSP_VERT_RES);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = PSP_BUF_WIDTH;//PSP_HOR_RES;
    disp_drv.ver_res = PSP_VERT_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_3;
    disp_drv.full_refresh = 1;

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    sceGuInit();
    sceGuStart(GU_DIRECT, DisplayList);
    sceGuDrawBuffer(GU_PSM_8888, vrelptr(draw_buf), PSP_BUF_WIDTH);
    sceGuDispBuffer(PSP_HOR_RES, PSP_VERT_RES, vrelptr(disp_buf), PSP_BUF_WIDTH);

    sceGuOffset(2048 - (PSP_HOR_RES>>1), 2048 - (PSP_VERT_RES>>1));
    sceGuViewport(2048, 2048, PSP_HOR_RES, PSP_VERT_RES);

    sceGuDisable(GU_DEPTH_TEST);

    // Scissoring 
    sceGuScissor(0, 0, PSP_HOR_RES, PSP_VERT_RES);
    sceGuEnable(GU_SCISSOR_TEST);

    sceGuFinish();
    sceGuSync(0,0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    sceDisplayWaitVblankCB();
    disp_buf = draw_buf;
    draw_buf = vabsptr(sceGuSwapBuffers());
    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
