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

#include "cam.h"
#include "x264_encode.h"


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
    return 0;
}