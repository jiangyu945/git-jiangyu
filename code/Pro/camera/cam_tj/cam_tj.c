/*
 ============================================================================
 Name        : camera.c
 Author      : Greein-jy
 Version     : v1.0
 date        : 2018-10-22
 Description : V4L2 capture video,add yuyv_2_yuv420p and yuyv_2_rgb,add rgb compress to jpg
 ============================================================================
 */

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
#include <sys/select.h>

#include <asm/types.h>
#include <linux/videodev2.h>

#include "turbojpeg.h"

#define  DEV_NAME  "/dev/video5"

#define  WIDTH    1920   
#define  HEIGHT   1080

typedef unsigned char uchar;

struct buffer
{
    void *start;
    size_t length;
};

struct buffer *buffers;
unsigned long n_buffers;
//unsigned long file_length;

static int cam_fd;

/*获取当前ms数*/
static int get_time_now()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return(now.tv_sec * 1000 + now.tv_usec / 1000);
}

int usTimer(long us)
{
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = us;

    return select(0,NULL,NULL,NULL,&timeout);
}


int tj_CompressFromMem(void* fp_jpg,uchar* rgb_buffer,int quality)
{
  tjhandle handle = NULL;
  int flags = 0;
  int pad = 4;   //字节对齐
  int subsamp = TJSAMP_422;
  int pixelfmt = TJPF_RGB;
  /*创建一个turbojpeg句柄*/
  handle=tjInitCompress();
  if (NULL == handle){
    return -1;
  }

  uchar *outjpg_buf=NULL;
  unsigned long outjpg_size;

  /*压缩*/
   //int ret = tjCompressFromYUV(handle,yuv_buffer,WIDTH,pad,HEIGHT,subsamp,&outjpg_buf,&outjpg_size,quality, flags);
  int ret = tjCompress2(handle, rgb_buffer,WIDTH,0,HEIGHT,pixelfmt,&outjpg_buf,&outjpg_size,subsamp,quality, flags);
  if (0 != ret) {
    tjDestroy(handle);
    return -1;
  }
  tjDestroy(handle);
 
  fwrite(outjpg_buf,1,outjpg_size,fp_jpg);

  return 0;
}


void yuyv_to_rgb(uchar* yuyv,uchar* rgb)
{
    unsigned int i;
    uchar* y0 = yuyv + 0;   
    uchar* u0 = yuyv + 1;
    uchar* y1 = yuyv + 2;
    uchar* v0 = yuyv + 3;

    uchar* r0 = rgb + 0;
    uchar* g0 = rgb + 1;
    uchar* b0 = rgb + 2;
    uchar* r1 = rgb + 3;
    uchar* g1 = rgb + 4;
    uchar* b1 = rgb + 5;
   
    float rt0 = 0, gt0 = 0, bt0 = 0, rt1 = 0, gt1 = 0, bt1 = 0;

    for(i = 0; i <= (WIDTH * HEIGHT) / 2 ;i++)
    {
        bt0 = 1.164 * (*y0 - 16) + 2.018 * (*u0 - 128); 
        gt0 = 1.164 * (*y0 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128); 
        rt0 = 1.164 * (*y0 - 16) + 1.596 * (*v0 - 128); 
   
    	bt1 = 1.164 * (*y1 - 16) + 2.018 * (*u0 - 128); 
        gt1 = 1.164 * (*y1 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128); 
        rt1 = 1.164 * (*y1 - 16) + 1.596 * (*v0 - 128); 
      
       	if(rt0 > 250)  	rt0 = 255;
		if(rt0< 0)    	rt0 = 0;	

		if(gt0 > 250) 	gt0 = 255;
		if(gt0 < 0)	gt0 = 0;	

		if(bt0 > 250)	bt0 = 255;
		if(bt0 < 0)	bt0 = 0;	

		if(rt1 > 250)	rt1 = 255;
		if(rt1 < 0)	rt1 = 0;	

		if(gt1 > 250)	gt1 = 255;
		if(gt1 < 0)	gt1 = 0;	

		if(bt1 > 250)	bt1 = 255;
		if(bt1 < 0)	bt1 = 0;	
					
		*r0 = (uchar)rt0;
		*g0 = (uchar)gt0;
		*b0 = (uchar)bt0;
	
		*r1 = (uchar)rt1;
		*g1 = (uchar)gt1;
		*b1 = (uchar)bt1;

        yuyv = yuyv + 4;
        rgb = rgb + 6;
        if(yuyv == NULL)
          break;

        y0 = yuyv;
        u0 = yuyv + 1;
        y1 = yuyv + 2;
        v0 = yuyv + 3;
  
        r0 = rgb + 0;
        g0 = rgb + 1;
        b0 = rgb + 2;
        r1 = rgb + 3;
        g1 = rgb + 4;
        b1 = rgb + 5;
    }   
}

int yuyv_to_yuv420p(const uchar *yuyv, uchar *yuv420p, unsigned int width, unsigned int height)
{
    
    uchar *y = yuv420p;
    uchar *u = yuv420p + width*height;
    uchar *v = yuv420p + width*height + width*height/4;

    unsigned int i,j;
    unsigned int base_h;
    unsigned int is_y = 1, is_u = 1;
    unsigned int y_index = 0, u_index = 0, v_index = 0;

    unsigned long yuv422_length = 2 * width * height;

    //序列为YU YV YU YV，一个yuv422帧的长度 width * height * 2 个字节
    //丢弃偶数行 u v

    for(i=0; i<yuv422_length; i+=2)
    {
        *(y+y_index) = *(yuyv+i);
        y_index++;
    }

    for(i=0; i<height; i+=2)
    {
        base_h = i*width*2;
        for(j=base_h+1; j<base_h+width*2; j+=2)
        {
            if(is_u){
                *(u+u_index) = *(yuyv+j);
                u_index++;
                is_u = 0;
            }
            else{
                *(v+v_index) = *(yuyv+j);
                v_index++;
                is_u = 1;
            }
        }
    }

    return 1;
}


static int open_cam()
{
    struct v4l2_capability cap;
    cam_fd = open(DEV_NAME, O_RDWR | O_NONBLOCK, 0);  //非阻塞方式打开摄像头
    if (cam_fd < 0)
    {
        perror("open device failed!");
        return -1;
    }

    /*获取摄像头信息*/
    if (ioctl(cam_fd, VIDIOC_QUERYCAP, &cap) < 0)
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
    return 0;
}

static int set_cap_frame()
{
    struct v4l2_format fmt;
    /*设置摄像头捕捉帧格式及分辨率*/
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;   
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;   //图像存储格式设为YUYV(YUV422)

    if (ioctl(cam_fd, VIDIOC_S_FMT, &fmt) < 0)
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

    return 0;
}

void get_fps()
{
    struct v4l2_streamparm *parm;
    parm = (struct v4l2_streamparm *)calloc(1, sizeof(struct v4l2_streamparm));
    memset(parm, 0, sizeof(struct v4l2_streamparm));
    parm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 

    int rel;
    rel = ioctl(cam_fd,VIDIOC_G_PARM, parm);

    if(rel == 0)
    {
        printf("Frame rate:  %u/%u\n",parm->parm.capture.timeperframe.denominator,parm->parm.capture.timeperframe.numerator);
    }
    free(parm);
}

static void init_mmap()
{
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    
    /*申请4个图像缓冲区（位于内核空间）*/
    req.count = 4;    //缓存数量，即缓存队列里保持多少张照片，一般不超过5个  (经测试此版本内核最多申请23个缓存区)
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP; //MMAP内存映射方式
    ioctl(cam_fd, VIDIOC_REQBUFS, &req);

    printf("req.count:%d\n",req.count);

    buffers = calloc(req.count, sizeof(struct buffer));
  
    /*将申请的4个缓冲区映射到用户空间*/
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        ioctl(cam_fd, VIDIOC_QUERYBUF, &buf);

        buffers[n_buffers].length = buf.length;

        buffers[n_buffers].start = mmap(NULL,
                                        buf.length,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED,
                                        cam_fd,
                                        buf.m.offset);
    }
    
    /*缓冲区入队列*/
    int i;
    for (i = 0; i < n_buffers; ++i)
    {
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        ioctl(cam_fd, VIDIOC_QBUF, &buf);
    }
 
}


static void start_cap()
{   
    /*使能视频设备输出视频流*/
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(cam_fd, VIDIOC_STREAMON, &type);
}

//图像数据处理
static void process_img(void* addr,int length,void* fp_jpg)
{ 
    /*
    uchar *yuyv_buf = (uchar *)malloc(2*WIDTH*HEIGHT*sizeof(uchar));
	uchar *yuv420_buf = (uchar *)malloc(3*WIDTH*HEIGHT/2*sizeof(uchar)); 
    uchar *rgb_buf = (uchar *)malloc(WIDTH*HEIGHT*3*sizeof(uchar)); 

    fwrite(addr,length,1,fp_yuyv); 
    
    fseek(fp_yuyv,-WIDTH*HEIGHT*2,SEEK_END);
    fread(yuyv_buf, WIDTH*HEIGHT*2, 1, fp_yuyv);

    yuyv_to_yuv420p(yuyv_buf,yuv420_buf,WIDTH,HEIGHT);
    fwrite(yuv420_buf, WIDTH*HEIGHT*3/2, 1, fp_yuv420);


    yuyv_to_rgb(yuyv_buf,rgb_buf);
    fwrite(rgb_buf, WIDTH*HEIGHT*3, 1, fp_rgb);

    //turbo-jpeg压缩
    int ret;
    ret = tj_CompressFromMem(rgb_buf,80);
    if(ret != 0){
        printf("compress failed!\n");
    }

    free(rgb_buf);
    free(yuyv_buf); 
    free(yuv420_buf);   
    */
    //uchar *yuv420_buf = (uchar *)malloc(3*WIDTH*HEIGHT/2*sizeof(uchar)); 
    uchar *rgb_buf = (uchar *)malloc(WIDTH*HEIGHT*3*sizeof(uchar));

    

    //yuyv_to_yuv420p(addr,yuv420_buf,WIDTH,HEIGHT);
    yuyv_to_rgb(addr,rgb_buf);
    int ret;
    ret = tj_CompressFromMem(fp_jpg,rgb_buf,90);
    if(ret != 0){
        printf("compress failed!\n");
    }

     //free(yuv420_buf); 
     free(rgb_buf);
}

/*获取一帧图像*/
static int read_frame(void* fp_jpg)
{
    struct v4l2_buffer buf;

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    ioctl(cam_fd, VIDIOC_DQBUF, &buf);  //出队，从队列中取出一个缓冲帧

    process_img(buffers[buf.index].start,buffers[buf.index].length,fp_jpg);
    
    ioctl(cam_fd, VIDIOC_QBUF, &buf); //重新入队

    return 1;

}

static void main_loop()
{
    //FILE* fp_yuyv = fopen("yuyv_1080p.yuv", "a+");  //以追加的方式打开
    //FILE* fp_yuv420 = fopen("yuv420p_1080p.yuv", "a+");  
   // FILE* fp_rgb = fopen("rgb_1080p.rgb", "a+");  
    FILE *fp_jpg = fopen("./tjout.jpg","ab+");
    if(NULL == fp_jpg){
        return;
    } 

    int start_t = get_time_now();

    int count; 
    for(count=1;count<=1;count++)
    {
        int ret;
        /*select监听*/
        fd_set fds;
        struct timeval timeout;      
        FD_ZERO(&fds);
        FD_SET(cam_fd, &fds);
        ret = select(cam_fd + 1, &fds, NULL, NULL, NULL);
        if(ret >0)
        {           
            read_frame(fp_jpg);
            printf("frame: %d\n",count);
                
        }
    }
 
    int end_t = get_time_now();
    printf("Total time-consuming: %dms\n",end_t-start_t);  
             
    //fclose(fp_yuyv);
    //fclose(fp_yuv420);
    //fclose(fp_rgb);  
    fclose(fp_jpg);
}

static void stop_cap()
{
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(cam_fd,VIDIOC_STREAMOFF,&type))
	{
		perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
		exit(EXIT_FAILURE);
	}
}


void close_cam()
{
    int i;
    //解除映射
    for (i = 0; i < n_buffers; ++i)
    {
        munmap(buffers[i].start, buffers[i].length);
    }

    free(buffers);   
    close(cam_fd);
    printf("Camera Capture Done.\n");
}

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
