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
  

int x264_encode(void* yuv_fd,void *h264_fd,int width,int height)
{       
         int ret;
         int y_size;
         int i,j;
         int csp=X264_CSP_I420;
  
         //Encode frame number
         //if set 0, encode all frame
         int frame_num=50;
         
         int iNal   = 0;
         x264_nal_t* pNals = NULL;
         x264_t* pHandle   = NULL;
         x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
         x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
         x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));
 
         x264_param_default(pParam);
         pParam->i_width   = width;   //宽度
         pParam->i_height  = height;  //高度
         
         //Param
         //pParam->i_log_level  = X264_LOG_DEBUG;
         pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;  
       //  pParam->i_frame_total = 0;
         pParam->i_keyint_max = 10;    //在此间隔设置IDR关键帧 
       //  pParam->i_bframe  = 5;
        // pParam->b_open_gop  = 0;
        // pParam->i_bframe_pyramid = 0;
        // pParam->rc.i_qp_constant=0;
        // pParam->rc.i_qp_max=0;
        // pParam->rc.i_qp_min=0;
        // pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;  
         pParam->i_fps_num  = 25;   //帧率
         pParam->i_fps_den  = 1;
        // pParam->i_timebase_den = pParam->i_fps_num;
        // pParam->i_timebase_num = pParam->i_fps_den;

         pParam->i_csp=csp;

         //set profile
         x264_param_apply_profile(pParam, x264_profile_names[5]);
         
         //open encoder
         pHandle = x264_encoder_open(pParam);
          
         x264_picture_init(pPic_out);
         x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);
 
         //ret = x264_encoder_headers(pHandle, &pNals, &iNal);
 
         y_size = pParam->i_width * pParam->i_height;

        //detect frame number
        if(frame_num==0){
                fseek(yuv_fd,0,SEEK_END);
                switch(csp){
                case X264_CSP_I444:frame_num=ftell(yuv_fd)/(y_size*3);break;
                case X264_CSP_I422:frame_num=ftell(yuv_fd)/(y_size*2);break;
                case X264_CSP_I420:frame_num=ftell(yuv_fd)/(y_size*3/2);break;
                default:printf("Colorspace Not Support.\n");return -1;
                }
                fseek(yuv_fd,0,SEEK_SET);
        }

        //Loop to Encode
        for( i=0;i<frame_num;i++){
                        switch(csp){
                        case X264_CSP_I444:{
                                fread(pPic_in->img.plane[0],y_size,1,yuv_fd);         //Y
                                fread(pPic_in->img.plane[1],y_size,1,yuv_fd);         //U
                                fread(pPic_in->img.plane[2],y_size,1,yuv_fd);         //V
                                break;}
                        case X264_CSP_I422:{
                                int index = 0;
                                int y_i=0,u_i=0,v_i=0;
                                for(index = 0 ; index < y_size*2 ;){
                                    fread(&pPic_in->img.plane[0][y_i++],1,1,yuv_fd);   //Y
                                    index++;
                                    fread(&pPic_in->img.plane[1][u_i++],1,1,yuv_fd);   //U
                                    index++;
                                    fread(&pPic_in->img.plane[0][y_i++],1,1,yuv_fd);   //Y
                                    index++;
                                    fread(&pPic_in->img.plane[2][v_i++],1,1,yuv_fd);   //V
                                    index++;
                                }break;}
                        case X264_CSP_I420:{
                                fread(pPic_in->img.plane[0],y_size,1,yuv_fd);         //Y
                                fread(pPic_in->img.plane[1],y_size/4,1,yuv_fd);       //U
                                fread(pPic_in->img.plane[2],y_size/4,1,yuv_fd);       //V
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
                            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, h264_fd);
                        }
        }
        i=0;

        //flush encoder
        while(1)
        {
            ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);
            if(ret==0){
                    break;
            }
            printf("Flush 1 frame.\n");
            for (j = 0; j < iNal; ++j){
                    fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, h264_fd);
            }
            i++;
        }

        x264_picture_clean(pPic_in);
        x264_encoder_close(pHandle);
        pHandle = NULL;
 
        free(pPic_in);
        free(pPic_out);
        free(pParam);
        
        return 0;
}
