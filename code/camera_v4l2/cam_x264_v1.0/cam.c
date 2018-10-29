#include "cam.h"
#include "x264_encode.h"


int yuyv_to_yuv420p(const unsigned char *in, unsigned char *out, unsigned int width, unsigned int height)
{
    
    unsigned char *y = out;
    unsigned char *u = out + width*height;
    unsigned char *v = out + width*height + width*height/4;

    unsigned int i,j;
    unsigned int base_h;
    unsigned int is_y = 1, is_u = 1;
    unsigned int y_index = 0, u_index = 0, v_index = 0;

    unsigned long yuv422_length = 2 * width * height;

    //序列为YU YV YU YV，一个yuv422帧的长度 width * height * 2 个字节
    //丢弃偶数行 u v

    for(i=0; i<yuv422_length; i+=2)
    {
        *(y+y_index) = *(in+i);
        y_index++;
    }

    for(i=0; i<height; i+=2)
    {
        base_h = i*width*2;
        for(j=base_h+1; j<base_h+width*2; j+=2)
        {
            if(is_u){
                *(u+u_index) = *(in+j);
                u_index++;
                is_u = 0;
            }
            else{
                *(v+v_index) = *(in+j);
                v_index++;
                is_u = 1;
            }
        }
    }

    return 1;
}


int open_cam()
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

int set_cap_frame()
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

void init_mmap()
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


void start_cap()
{   
    /*使能视频设备输出视频流*/
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(cam_fd, VIDIOC_STREAMON, &type);
}

void process_img(void* addr,int length,void* fp_yuyv,void* fp_yuv420)
{   
    unsigned char *yuyv_buf = (unsigned char *)malloc(2*1280*720*sizeof(unsigned char));
	unsigned char *yuv420_buf = (unsigned char *)malloc(3*1280*720/2*sizeof(unsigned char)); 

    fwrite(addr,length,1,fp_yuyv); 
    
    fseek(fp_yuyv,-WIDTH*HEIGHT*2,SEEK_END);
    fread(yuyv_buf, WIDTH*HEIGHT*2, 1, fp_yuyv);

    yuyv_to_yuv420p(yuyv_buf,yuv420_buf,WIDTH,HEIGHT);
    fwrite(yuv420_buf, WIDTH*HEIGHT*3/2, 1, fp_yuv420);

    free(yuyv_buf); 
    free(yuv420_buf);   
 
}

/*获取一帧图像*/
int read_frame(void* fp_yuyv,void* fp_yuv420)
{
    struct v4l2_buffer buf;

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    ioctl(cam_fd, VIDIOC_DQBUF, &buf);  //出队，从队列中取出一个缓冲帧

    process_img(buffers[buf.index].start,buffers[buf.index].length,fp_yuyv,fp_yuv420);
    
    ioctl(cam_fd, VIDIOC_QBUF, &buf); //重新入队

    return 1;

}

void main_loop()
{   
    FILE* fp_yuyv = fopen("yuyv_640x480.yuv", "a+");  //以追加的方式打开
    FILE* fp_yuv420 = fopen("yuv420p_640x480.yuv", "a+");  //以追加的方式打开
    FILE* fp_h264 = fopen("yuv420p_640x480.h264", "a+");  //以追加的方式打开


    int count=1; 
    for(;count<51;count++)
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
            read_frame(fp_yuyv,fp_yuv420);
            printf("frame: %d\n",count);


            if( count%50 == 0 ){
                fseek(fp_yuv420,-(WIDTH*HEIGHT*3/2)*50,SEEK_END);
                x264_encode(fp_yuv420,fp_h264,WIDTH,HEIGHT);
            }
            /*
            gettimeofday (&tv, &tz);
            printf("tv_sec; %d\n", tv.tv_sec);
            printf("tv_usec; %d\n", tv.tv_usec); 
            */
        }
    }
    fclose(fp_yuyv);  
    fclose(fp_yuv420);
    fclose(fp_h264);
}

void stop_cap()
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


