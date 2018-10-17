/*
 ============================================================================
 Name        : main.c
 Author      : Greein-jy
 Version     : v1.0
 date        : 2018-10-16
 Description : V4L2 capture video and encode by libx264
 ============================================================================
 */


#include <stdio.h>

#include "camera.h"
#include "x264_coder.h"


int main(int argc, char **argv)
{
    open_cam();
    set_cap_frame();
    get_fps();
    init_mmap();
    start_cap();
    main_loop();
    stop_cap();
    close_cam();
    x264_code();  //编码压缩
    return 0;
}