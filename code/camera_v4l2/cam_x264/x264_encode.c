/**
 * 最简单的基于X264的视频编码器
 * Simplest X264 Encoder
 * jiangyu
 * 
 * 本程序可以YUV格式的像素数据编码为H.264码流，是最简单的
 * 基于libx264的视频编码器
 *
 * This software encode YUV data to H.264 bitstream.
 * It's the simplest encoder example based on libx264.
 */
#include "x264_encode.h"

#include "net.h"  

int x264_encode(unsigned char* yuv420_buf,void *fp_h264,int width,int height)
{       
         int ret;
         int y_size;
         int i,j;
         int csp=X264_CSP_I420;
  
         //Encode frame number
         //if set 0, encode all frame
         int frame_num=1;
         
         int iNal   = 0;
         x264_nal_t* pNals = NULL;
         x264_t* pHandle   = NULL;
         x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
         x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
 
        
         
         x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));
         //编码延迟。 设置zerolatency参数，达到即时编码      
         x264_param_default_preset(pParam, "fast" , "zerolatency" );  

         //编码比特流的CSP,仅支持i420，色彩空间设置
         pParam->i_csp=csp;
         
         //分辨率
         pParam->i_width   = width;  
         pParam->i_height  = height; 
         
         //编码算术
         pParam->b_cabac = 1;   //cabac：上下文自适应二进制算术编码，在high档次及其以上才能支持

         //多线程并行编码        
         pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;   //设为auto时，线程数为逻辑CPU个数的1.5倍。

         //rc
         pParam->rc.b_mb_tree = 0;           //这个不为0,将导致编码延时帧,在实时编码时,必须为0
         //图像质量控制
         pParam->rc.f_rf_constant = 25;      //实际质量，越大图像越花，越小越清晰。
         pParam->rc.f_rf_constant_max = 45;  //图像质量的最大值

         //码率控制 
         pParam->rc.i_bitrate = 2000;            // 平均码率,在ABR(平均码率)模式下才生效，且必须在设置ABR前先设置bitrate（单位kbps，此k为1000，而不是1024），1080p推荐4Mbps码率
         pParam->rc.i_vbv_max_bitrate=2000*1.2;  // 平均码率模式下，最大瞬时码率，默认0(与-B设置相同)
         pParam->rc.i_rc_method = X264_RC_ABR;   // 码率控制方法，CQP(恒定质量)，CRF(恒定码率,缺省值23)，ABR(平均码率)，VBR（动态比特率）


         //使用实时视频传输时，需要实时发送sps,pps数据
         pParam->b_repeat_headers = 1;       //在每个关键帧I帧前添加sps和pps，实时视频传输需要enable

         // I帧间隔
         pParam->b_vfr_input = 0;        // 1:使用timebase和时间戳做码率控制 0：只使用fps做码率控制
         pParam->i_fps_num  = 25;        // 设置帧率（分子）
         pParam->i_fps_den  = 1;         // 设置帧率时间1s（分母）
         pParam->i_keyint_max = 25;      //IDR关键帧间隔设置,尽量保证每秒钟视频至少包含一个关键帧！！播放帧率25fps的话，最大设置成25
         //pParam->i_timebase_den = 1;    //timebase（分子）
         //pParam->i_timebase_num = 1000;    //timebase（分母）

         /*
         pParam->b_annexb = 0;//如果设置了该项，则在每个NAL单元前加一个四字节的前缀符
         pParam->i_log_level  = X264_LOG_DEBUG;  //LOG参数
         pParam->i_frame_total = 0;   //编码总帧数，默认0
         pParam->i_bframe  = 5;
         pParam->b_open_gop  = 0;
         pParam->rc.i_qp_constant=0;   //0-51质量
         pParam->rc.i_qp_max=0;        //允许的最大量化值
         pParam->rc.i_qp_min=0;        //允许的最小量化值
         pParam->i_bframe_pyramid = 0;
         pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
         */
         //set profile
         x264_param_apply_profile(pParam, "baseline");  //baseline
         
         //编码复杂度
         pParam->i_level_idc=30;    
         
         //open encoder
         pHandle = x264_encoder_open(pParam);
          
         x264_picture_init(pPic_out);
         x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);
 
         //ret = x264_encoder_headers(pHandle, &pNals, &iNal);
 
         y_size = pParam->i_width * pParam->i_height;

        //Loop to Encode
        for( i=0;i<frame_num;i++){
                        switch(csp){
                        case X264_CSP_I420:{
                                memcpy(pPic_in->img.plane[0],yuv420_buf,y_size);                  //Y
                                memcpy(pPic_in->img.plane[1],&yuv420_buf[y_size],y_size/4);       //U
                                memcpy(pPic_in->img.plane[2],&yuv420_buf[y_size*5/4],y_size/4);   //V
                                break;}
                        default:{
                                printf("Colorspace Not Support.\n");
                                return -1;}
                        }
                    
                        ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
                        if (ret< 0){
                            printf("Error.\n");
                            return -1;
                        }

                        for ( j = 0; j < iNal; ++j){
                            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_h264);
                            //send(connfd,pNals[j].p_payload,pNals[j].i_payload,0);
                        }
        }

        x264_picture_clean(pPic_in);
        x264_encoder_close(pHandle);
        pHandle = NULL;
 
        free(pPic_in);
        free(pPic_out);
        free(pParam);
        
        return 0;
}
