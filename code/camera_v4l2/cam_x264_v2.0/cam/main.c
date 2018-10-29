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

#include "cam.h"
#include "x264_encode.h"
#include  "net.h"



void main_loop()
{   
    FILE* fp_yuyv = fopen("yuyv_640x480.yuv", "a+");  //以追加的方式打开
    FILE* fp_h264 = fopen("yuv_640x480.h264", "a+");  //以追加的方式打开

    unsigned char *yuyv_buf = (unsigned char *)malloc(2*1280*720*sizeof(unsigned char));
	unsigned char *yuv420_buf = (unsigned char *)malloc(3*1280*720/2*sizeof(unsigned char)); 

    int count;
    for(count=1;count<=100;count++)
    {
        int ret;
        /*select监听*/
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(cam_fd, &fds);
        ret = select(cam_fd + 1, &fds, NULL, NULL, NULL);
        if(ret >0)
        {
            /*//获取当前时间
            struct timeval tv;
            struct timezone tz;
            gettimeofday (&tv, &tz);
            printf("tv_sec; %d\n", tv.tv_sec);
            printf("tv_usec; %d\n", tv.tv_usec);
            */
            //read one frame
            read_frame(fp_yuyv);
            
            fseek(fp_yuyv,-WIDTH*HEIGHT*2,SEEK_END);
            fread(yuyv_buf, WIDTH*HEIGHT*2, 1, fp_yuyv);

            yuyv_to_yuv420p(yuyv_buf,yuv420_buf,WIDTH,HEIGHT);

            //encode 
            x264_encode(yuv420_buf,fp_h264,WIDTH,HEIGHT);

            memset(yuyv_buf,0,sizeof(yuyv_buf));
            memset(yuv420_buf,0,sizeof(yuv420_buf));    
        }
    }

    free(yuyv_buf); 
    free(yuv420_buf); 
    fclose(fp_yuyv); 
    fclose(fp_h264); 
}


int main(int argc, char **argv)
{
    init_net();
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