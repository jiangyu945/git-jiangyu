#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>
#include <linux/videodev2.h>

#define DEV_NAME  "/dev/video4"
 //分辨率
#define  WIDTH   1920           
#define  HEIGHT  1080

struct buffer
{
    void *start;
    size_t length;
};

struct buffer *buffers;
unsigned char* rgb_buf;
rgb_buf = malloc(WIDTH*HEIGHT*3);
unsigned long n_buffers;
unsigned long file_length;

int file_fd;

int fd;

//YVUV(YUV422)转RGB24
static void yuyv_to_rgb24(unsigned char* yuv_buf,unsigned char* rgb_buf)
{
    unsigned int i;
    unsigned char* y0 = yuv + 0;   
    unsigned char* u0 = yuv + 1;
    unsigned char* y1 = yuv + 2;
    unsigned char* v0 = yuv + 3;

    unsigned  char* r0 = rgb + 0;
    unsigned  char* g0 = rgb + 1;
    unsigned  char* b0 = rgb + 2;
    unsigned  char* r1 = rgb + 3;
    unsigned  char* g1 = rgb + 4;
    unsigned  char* b1 = rgb + 5;

    float rt0 = 0, gt0 = 0, bt0 = 0, rt1 = 0, gt1 = 0, bt1 = 0;

    for(i = 0; i <= (WIDTH * HEIGHT) / 2 ;i++)
    {
        bt0 = 1.164 * (*y0 - 16) + 2.018 * (*u0 - 128); 
        gt0 = 1.164 * (*y0 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128); 
        rt0 = 1.164 * (*y0 - 16) + 1.596 * (*v0 - 128); 

        bt1 = 1.164 * (*y1 - 16) + 2.018 * (*u0 - 128); 
        gt1 = 1.164 * (*y1 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128); 
        rt1 = 1.164 * (*y1 - 16) + 1.596 * (*v0 - 128); 
    
    
        if(rt0 > 250)     rt0 = 255;
        if(rt0< 0)        rt0 = 0;    

        if(gt0 > 250)     gt0 = 255;
        if(gt0 < 0)       gt0 = 0;    

        if(bt0 > 250)     bt0 = 255;
        if(bt0 < 0)       bt0 = 0;    

        if(rt1 > 250)     rt1 = 255;
        if(rt1 < 0)       rt1 = 0;    

        if(gt1 > 250)     gt1 = 255;
        if(gt1 < 0)       gt1 = 0;    

        if(bt1 > 250)     bt1 = 255;
        if(bt1 < 0)       bt1 = 0;    
                    
        *r0 = (unsigned char)rt0;
        *g0 = (unsigned char)gt0;
        *b0 = (unsigned char)bt0;
    
        *r1 = (unsigned char)rt1;
        *g1 = (unsigned char)gt1;
        *b1 = (unsigned char)bt1;

        yuv = yuv + 4;
        rgb = rgb + 6;
        if(yuv == NULL)
        break;

        y0 = yuv;
        u0 = yuv + 1;
        y1 = yuv + 2;
        v0 = yuv + 3;

        r0 = rgb + 0;
        g0 = rgb + 1;
        b0 = rgb + 2;
        r1 = rgb + 3;
        g1 = rgb + 4;
        b1 = rgb + 5;
    } 
    printf("YUYV to RGB24 end!\n"); 
}


/*获取一帧图像*/
static int read_frame(void)
{
    struct v4l2_buffer buf;

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    ioctl(fd, VIDIOC_DQBUF, &buf);  //出队，从队列中取出一个缓冲帧

    yuyv_to_rgb24(buffers[buf.index].start,rgb_buffers);   //YVUV(YUV422)转RGB24
    write(file_fd, buffers[buf.index].start, buffers[buf.index].length);  //将图片写入图像件“test.jpg”

    ioctl(fd, VIDIOC_QBUF, &buf); //重新入队

    return 1;

}

int main(int argc, char **argv)
{
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;

    file_fd = open("test.jpg", O_RDWR | O_CREAT, 0777);
    fd = open(DEV_NAME, O_RDWR | O_NONBLOCK, 0);  //非阻塞方式打开摄像头
    if (fd < 0)
    {
        perror("open device failed!");
        return -1;
    }

    /*获取摄像头信息*/
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        perror("get info failed!");
        return -1;
    }
    printf("Driver Name:%s\n Card Name:%s\n Bus info:%s\n version:%d\n capabilities:%x\n \n ", cap.driver, cap.card, cap.bus_info,cap.version,cap.capabilities);
             
    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
    {
        printf("Device %s: supports capture.\n",DEV_NAME);
    }
    if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
    {
        printf("Device %s: supports streaming.\n",DEV_NAME);
    }
    

    /*设置摄像头捕捉帧格式及分辨率*/
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;   //帧宽
    fmt.fmt.pix.height = HEIGHT;  //帧高
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;   //图像存储格式设为YUYV(YUV422)

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        perror("set fmt failed!");
        return -1;
    }

    printf("fmt.type:%d\n",fmt.type);
    printf("pix.pixelformat:%c%c%c%c\n", \
            fmt.fmt.pix.pixelformat & 0xFF,\
            (fmt.fmt.pix.pixelformat >> 8) & 0xFF, \
            (fmt.fmt.pix.pixelformat >> 16) & 0xFF,\
            (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    printf("pix.width:%d\n",fmt.fmt.pix.width);
    printf("pix.height:%d\n",fmt.fmt.pix.height);
    printf("pix.field:%d\n",fmt.fmt.pix.field);

    /*申请4个图像缓冲区（位于内核空间）*/
    req.count = 4;    //缓存数量，即缓存队列里保持多少张照片，一般不超过5个  (经测试此版本内核最多申请23个缓存区)
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP; //MMAP内存映射方式
    ioctl(fd, VIDIOC_REQBUFS, &req);

    printf("req.count:%d\n",req.count);

    buffers = calloc(req.count, sizeof(*buffers));

    /*将申请的4个缓冲区映射到用户空间*/
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        ioctl(fd, VIDIOC_QUERYBUF, &buf);

        buffers[n_buffers].length = buf.length;

        buffers[n_buffers].start = mmap(NULL,
                                        buf.length,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED,
                                        fd,
                                        buf.m.offset);
    }

    /*缓冲区入队列*/
    int i;
    for (i = 0; i < n_buffers; ++i)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    /*使能视频设备输出视频流*/
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);

    /*select监听*/
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    select(fd + 1, &fds, NULL, NULL, NULL);

    read_frame();
    
    for (i = 0; i < n_buffers; ++i)
        munmap(buffers[i].start, buffers[i].length);

    close(fd);
    close(file_fd);
    printf("Camera Capture Done.\n");

    return 0;

}
