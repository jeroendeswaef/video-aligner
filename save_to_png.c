
// Code based on a tutorial: http://dranger.com/ffmpeg/tutorial01.html
//
// Compile:
// gcc -o save_to_png save_to_png.c -I/usr/include/ffmpeg -lavutil -lavformat -lavcodec -lz -lavutil -lm -lswscale

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <stdio.h>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#define CHECK_ERR(ERR) {if ((ERR)<0) return -1; }

int main(int argc, char *argv[]) {
	// Initalizing these to NULL prevents segfaults!
	AVFormatContext   *pFormatCtx = NULL;
	int               i, videoStream;
	AVCodecContext    *pCodecCtxOrig = NULL;
	AVCodecContext    *pCodecCtx = NULL;
	AVCodec           *pCodec = NULL;
	AVFrame           *pFrame = NULL;
	AVFrame           *pFrameRGB = NULL;
	AVPacket          packet;
	int               frameFinished;
	int               numBytes;
	uint8_t           *buffer = NULL;
	struct SwsContext *sws_ctx = NULL;

	if(argc < 2) {
		printf("Please provide a movie file\n");
		return -1;
	}
	// Register all formats and codecs
	av_register_all();

	// Open video file
	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
		return -1; // Couldn't open file

	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, argv[1], 0);

	// Find the first video stream
	videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			videoStream=i;
			break;
		}
	if(videoStream==-1)
		return -1; // Didn't find a video stream

	// Get a pointer to the codec context for the video stream
	pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
	// Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if(pCodec==NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}
	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Open codec
	if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
		return -1; // Could not open codec

	// Allocate video frame
	pFrame=av_frame_alloc();

	// Allocate an AVFrame structure
	pFrameRGB=av_frame_alloc();
	if(pFrameRGB==NULL)
		return -1;

	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
			pCodecCtx->height);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
			pCodecCtx->width, pCodecCtx->height);

	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(pCodecCtx->width,
			pCodecCtx->height,
			pCodecCtx->pix_fmt,
			pCodecCtx->width,
			pCodecCtx->height,
			PIX_FMT_RGB24,
			SWS_BILINEAR,
			NULL,
			NULL,
			NULL
			);
	int err;

	for (;;)
	{
		err = av_read_frame(pFormatCtx, &packet);
		CHECK_ERR(err);
		if (packet.stream_index == videoStream)
		{
			int got = 0;
			AVFrame * frame = av_frame_alloc();
			err = avcodec_decode_video2(pCodecCtx, frame, &got, &packet);
			CHECK_ERR(err);

			if (got)
			{
				AVFrame * rgbFrame = av_frame_alloc();
				avpicture_alloc((AVPicture *)rgbFrame, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
				sws_scale(sws_ctx, (uint8_t const * const *) frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize);

				AVCodec *outCodec = avcodec_find_encoder(CODEC_ID_PNG);
				AVCodecContext *outCodecCtx = avcodec_alloc_context3(outCodec);
				if (!pCodecCtx) 
					return -1;                  

				outCodecCtx->width = pCodecCtx->width;
				outCodecCtx->height = pCodecCtx->height;
				outCodecCtx->pix_fmt = PIX_FMT_RGB24;
				outCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
				outCodecCtx->time_base.num = pCodecCtx->time_base.num;
				outCodecCtx->time_base.den = pCodecCtx->time_base.den;

				
				if (!outCodec || avcodec_open2(outCodecCtx, outCodec, NULL) < 0) {
					return -1;
				}
				AVPacket outPacket;
				av_init_packet(&outPacket);
				outPacket.size = 0;
				outPacket.data = NULL;
				int gotFrame = 0;
				int ret = avcodec_encode_video2(outCodecCtx, &outPacket, rgbFrame, &gotFrame);
				if (ret >= 0 && gotFrame)
				{
					FILE * outPng = fopen("tst.png", "wb");
					fwrite(outPacket.data, outPacket.size, 1, outPng);
					fclose(outPng);
				}

				avcodec_close(outCodecCtx);
				av_free(outCodecCtx);

				break;
			}
			av_frame_free(&frame);
		}
	} 
	// Free the RGB image
	av_free(buffer);
	av_frame_free(&pFrameRGB);

	// Free the YUV frame
	av_frame_free(&pFrame);

	// Close the codecs
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	return 0;
}
