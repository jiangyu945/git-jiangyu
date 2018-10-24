#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
 
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"

 
#define INBUF_SIZE 4096*8

typedef struct{
	AVCodec			*pCodec;			
	AVCodecContext	*pCodecContext;			
	AVCodecParserContext *pCodecParserCtx;	
	AVFrame			*frame;					
	AVPacket		pkt;					
} CodecCtx;
 
typedef struct{
	FILE *pFin;
	FILE *pFout;
	char *pNameIn;
	char *pNameOut;
} IOParam;
 
int Open_deocder(CodecCtx ctx) {
	avcodec_register_all();
	av_init_packet(&(ctx.pkt));
	ctx.pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!ctx.pCodec)	return -1;
 
	ctx.pCodecContext = avcodec_alloc_context3(ctx.pCodec);
	if (!ctx.pCodecContext)	return -2;
 
	if (ctx.pCodec->capabilities & AV_CODEC_CAP_TRUNCATED)
		ctx.pCodecContext->flags |= AV_CODEC_FLAG_TRUNCATED;
	ctx.pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264);
	if (!ctx.pCodecParserCtx)	return -3;
 
	if (avcodec_open2(ctx.pCodecContext, ctx.pCodec, NULL) < 0)
		return -4;
 
	ctx.frame = av_frame_alloc();
	if (!ctx.frame)	return -5;
	return 0;
}
 
int Open_files(IOParam IOParam){
	IOParam.pFin = fopen(IOParam.pNameIn, "rb");
	if (!IOParam.pFin)	return -1;
	IOParam.pFout = fopen(IOParam.pNameOut, "wb");
	if (!IOParam.pFout)	return -2;
	return 0;
}
 
void Close_decoder(CodecCtx ctx) {
	avcodec_close(ctx.pCodecContext);
	av_free(ctx.pCodecContext);
	av_frame_free(&(ctx.frame));
}
 
void Close_files(IOParam IOParam){
	fclose(IOParam.pFin);
	fclose(IOParam.pFout);
}
 
void write_out_yuv_frame(const CodecCtx ctx, IOParam in_out){
	uint8_t **pBuf	= ctx.frame->data;
	int*	pStride = ctx.frame->linesize;
    int color_idx,idx;
	for (color_idx = 0; color_idx < 3; color_idx++){
		int	nWidth	= color_idx == 0 ? ctx.frame->width : ctx.frame->width / 2;
		int	nHeight = color_idx == 0 ? ctx.frame->height : ctx.frame->height / 2;
		for(idx=0;idx < nHeight; idx++){
			fwrite(pBuf[color_idx],1, nWidth, in_out.pFout);
			pBuf[color_idx] += pStride[color_idx];
		}
		fflush(in_out.pFout);
	}
}
 
void errReport(char *info, int val){
	fprintf(stderr, "ERR: %s code=%d\n",info, val);
	getch();
	exit(0);
}
 
int main(){
	int ret = 0;
	uint8_t *pDataPtr = NULL;
	int uDataSize = 0;
	int got_picture, len;
	CodecCtx ctx;
	IOParam inputoutput;
	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	inputoutput.pNameIn = "yuv420p_640x480.h264";
	inputoutput.pNameOut = "decode.yuv";
	if ((ret = Open_files(inputoutput)) < 0) 
		errReport("Open_files", ret);
 
	if ((ret = Open_deocder(ctx)) < 0) 
		errReport("Open_deocder", ret);
 
	printf("start...");
	while(1){
		uDataSize = fread_s(inbuf,INBUF_SIZE, 1, INBUF_SIZE, inputoutput.pFin);
		if (0 == uDataSize)	break;
		pDataPtr = inbuf;
		printf("\n%d:", uDataSize);
		while(uDataSize > 0){
			len = av_parser_parse2(ctx.pCodecParserCtx, ctx.pCodecContext, 
										&(ctx.pkt.data), &(ctx.pkt.size), 
										pDataPtr, uDataSize, 
										AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
			pDataPtr += len;
			uDataSize -= len;
			if (0 == ctx.pkt.size) 	continue;
 
			printf("#%d ", ctx.pkt.size);
			if ((ret = avcodec_decode_video2(ctx.pCodecContext, ctx.frame, &got_picture, &(ctx.pkt))) < 0)
				errReport("avcodec_decode_video2", ret);
 
			if (got_picture)
				write_out_yuv_frame(ctx, inputoutput);
		} //while(uDataSize > 0)
	}//while(1)
	printf("\navcodec_decode --- ok\n");
	Close_files(inputoutput);
	Close_decoder(ctx);
	getch();
	return 1;
}
