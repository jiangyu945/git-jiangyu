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

#define  DEV_NAME  "/dev/video5"

#define  WIDTH    640   
#define  HEIGHT   480


struct buffer
{
    void *start;
    size_t length;
};

struct buffer *buffers;
unsigned long n_buffers;
int cam_fd; 
  
int open_cam(void); 
int set_cap_frame(void);
void get_fps(void);
void init_mmap(void);
void start_cap(void);
int read_frame(void);
void main_loop(void);
void stop_cap(void);
void close_cam(void);



