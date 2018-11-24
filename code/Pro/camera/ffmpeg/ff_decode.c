/**基于雷神（雷霄骅）博文修改
 * 最简单的基于FFmpeg的视频解码器（纯净版）
 * Simplest FFmpeg Decoder Pure
 *
 * 本程序实现了视频码流(支持HEVC，H.264，MPEG2等)解码为YUV数据。
 * 它仅仅使用了libavcodec（而没有使用libavformat）。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest decoder based on FFmpeg.
 * It decode bitstreams to YUV pixel data.
 * It just use libavcodec (do not contains libavformat).
 * Suitable for beginner of FFmpeg.
 */
 
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
 
#define __STDC_CONSTANT_MACROS
 

 
int main(int argc, char* argv[])
{
	AVCodec *pCodec;
    
	const int in_buffer_size=4096;
	uint8_t in_buffer[4096 + 3110400]={0};
	uint8_t *cur_ptr;
	int cur_size;
	int ret,i,got_picture;
	int y_size;
    AVFrame	*pFrame;     //AVFrame存储一帧解码后的像素数据
    AVPacket packet;    //存储一帧（一般情况下）压缩编码数据
 
	enum AVCodecID codec_id=AV_CODEC_ID_H264;
	char filepath_in[]="yuv_1080p.h264";
	char filepath_out[]="decode_yuv420p_1080p.yuv";
	
	int first_time=1;
 	
	//注册所有的编解码器
	void *opaque = NULL;
	av_demuxer_iterate(&opaque);
	//av_register_all();  被弃用
	
	//打开多媒体文件
	AVFormatContext *pFormatCtx = NULL;
	//为AVFormatContext分配内存
	pFormatCtx=avformat_alloc_context();
	if(avformat_open_input(&pFormatCtx, filepath_in, NULL, NULL) != 0){
		return -1;   // Couldn't open file
	}
	//独立的解码上下文
	//AVCodecContext视频解码的上下文,为AVCodecContext分配内存
	AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx){
        printf("Could not allocate video codec context\n");
        return -1;
    }

	//循环遍历所有流，找到视频流
	int videoStream = -1; 
	for(i = 0; i < pFormatCtx->nb_streams; i++) { 
	if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) { 
			videoStream = i; 
			break; 
		} 
	}

	//将配置参数复制到AVCodecContext中
	avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);

	//查找视频解码器
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);  
    if (!pCodec) {
        printf("Codec not found\n");
        return -1;
    }

    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec\n");
        return -1;
    }
    	
	//初始化AVCodecParserContext
	AVCodecParserContext *pCodecParserCtx=NULL;
	pCodecParserCtx=av_parser_init(codec_id);
	if (!pCodecParserCtx){
		printf("Could not allocate video parser context\n");
		return -1;
	}

	//Input File
    FILE *fp_in = fopen(filepath_in, "rb");
    if (!fp_in) {
        printf("Could not open input stream\n");
        return -1;
    }

	//Output File
	FILE *fp_out = fopen(filepath_out, "wb");
	if (!fp_out) {
		printf("Could not open output YUV file\n");
		return -1;
	}
	
	//为AVFrame分配内存
    pFrame = av_frame_alloc();
	//初始化AVPacket
	av_init_packet(&packet);
 
	int n_frame=0;
	while (1) {
        cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
        if (cur_size == 0)
            break;
        cur_ptr=in_buffer;
 
        while (cur_size>0){
 
            //解析获得一个Packet
			int len = av_parser_parse2(
				pCodecParserCtx, pCodecCtx,
				&packet.data, &packet.size,
				cur_ptr , cur_size ,
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
 
			cur_ptr += len;
			cur_size -= len;
 
			if(packet.size==0)
				continue;
 
			//Some Info from AVCodecParserContext
			printf("[Packet]Size:%6d\t",packet.size);
			switch(pCodecParserCtx->pict_type){
				case AV_PICTURE_TYPE_I: printf("Type:I\t");break;
				case AV_PICTURE_TYPE_P: printf("Type:P\t");break;
				case AV_PICTURE_TYPE_B: printf("Type:B\t");break;
				default: printf("Type:Other\t");break;
			}
			printf("Number:%4d\n",pCodecParserCtx->output_picture_number);

			//解码一帧数据
			//avcodec_send_packet()向解码器提供输入AVPacket
			ret = avcodec_send_packet(pCodecCtx,  &packet); 
			if (ret != 0)
			{
				return;
			}
            //avcodec_receive_frame()接收解码的帧AVFrame
			while( avcodec_receive_frame(pCodecCtx, pFrame) == 0)
			{
				got_picture=1;
				if(got_picture){
					//读取到一帧视频，处理解码后视频frame
					if(first_time){
						printf("\nCodec Full Name:%s\n",pCodecCtx->codec->long_name);
						printf("width:%d\nheight:%d\n\n",pCodecCtx->width,pCodecCtx->height);
						first_time=0;
					}
					//Y, U, V
					int i;
					for(i=0;i<pFrame->height;i++){
						fwrite(pFrame->data[0]+pFrame->linesize[0]*i,1,pFrame->width,fp_out);
					}
					for(i=0;i<pFrame->height/2;i++){
						fwrite(pFrame->data[1]+pFrame->linesize[1]*i,1,pFrame->width/2,fp_out);
					}
					for(i=0;i<pFrame->height/2;i++){
						fwrite(pFrame->data[2]+pFrame->linesize[2]*i,1,pFrame->width/2,fp_out);
					}

					n_frame++;
					printf("Succeed to decode %d frame!\n",n_frame);			
				}
			}
		}
 
    }
 
	//Flush Decoder
    packet.data = NULL;
    packet.size = 0;
	while(1){
		ret = avcodec_receive_frame(pCodecCtx, pFrame);
		if (ret < 0) {
			printf("Decode Error.\n");
			return ret;
		}
		if (!got_picture){
			break;
		}else {
			//Y, U, V
			int i;
			for(i=0;i<pFrame->height;i++){
				fwrite(pFrame->data[0]+pFrame->linesize[0]*i,1,pFrame->width,fp_out);
			}
			for(i=0;i<pFrame->height/2;i++){
				fwrite(pFrame->data[1]+pFrame->linesize[1]*i,1,pFrame->width/2,fp_out);
			}
			for(i=0;i<pFrame->height/2;i++){
				fwrite(pFrame->data[2]+pFrame->linesize[2]*i,1,pFrame->width/2,fp_out);
			}
 
			printf("Flush Decoder: Succeed to decode 1 frame!\n");
		}
	}
 
    fclose(fp_in);
	fclose(fp_out);
 
 
	//释放AVFormatContext和它所有的流
    avformat_free_context(pFormatCtx);

	av_parser_close(pCodecParserCtx);


	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
 
	return 0;
}
