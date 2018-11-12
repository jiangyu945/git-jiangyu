/*
 ============================================================================
 Name        : main.c
 Author      : Greein-jy
 Version     : v2.0
 date        : 2018-10-29
 Description : V4L2 capture video and encode by libx264,then send to client and show.
 ============================================================================
 */
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include "x264_encode.h"
#include "cam.h"
#include "net.h"

/*获取当前ms数*/
static int get_time_now()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return(now.tv_sec * 1000 + now.tv_usec / 1000);
}


void main_loop()
{   
    //FILE* fp_yuyv = fopen("yuyv_1080p.yuv", "a+");  //以追加的方式打开 
    FILE* fp_h264 = fopen("yuv_1080p.h264", "a+");  
     
    int start_t = get_time_now();

    int count;
    for(count=1;count<=50;count++)
    {
        int ret;
        /*select监听*/
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(cam_fd, &fds);
        ret = select(cam_fd + 1, &fds, NULL, NULL, NULL);
        if(ret >0)
        {
            printf("start capture frame:  %d ... \n",count);
            //read one frame
            read_frame(fp_h264);
        
        }
    }

    int end_t = get_time_now();
    printf("Total time-consuming: %dms\n",end_t-start_t);

    fclose(fp_h264); 
}


int main(int argc, char **argv)
{
    
    //init_net();
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