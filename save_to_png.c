
// Code based on a tutorial: http://dranger.com/ffmpeg/tutorial01.html
//
// Compile:
// gcc -o save_to_png save_to_png.c -I/usr/include/ffmpeg -lavutil -lavformat -lavcodec -lz -lavutil -lm -lswscale

#ifdef __cplusplus
extern "C" {
#endif
//extern "C" { 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
//}
#ifdef __cplusplus
}
#endif
#include <stdio.h>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#define CHECK_ERR(ERR) {if ((ERR)<0) return -1; }

int video_aligner_get_frame(uint8_t **p_frame_image_data, int* p_width, int* p_height, int* p_linesize) {

	char const		  *p_video_filename = "test.mp4";

	// Initalizing these to NULL prevents segfaults!
	AVFormatContext   *pFormatCtx = NULL;
	int               i, videoStream;
	AVCodecContext    *pCodecCtxOrig = NULL;
	AVCodecContext    *pCodecCtx = NULL;
	AVCodec           *pCodec = NULL;
	AVFrame           *pFrame = NULL;
//	AVFrame           *pFrameRGB = NULL;
	AVFrame           *pFrameRGBResized = NULL;
	AVPacket          packet;
	int               frameFinished;
	int               numBytes;
	uint8_t           *buffer = NULL;
	struct SwsContext *sws_ctx = NULL;

	// Register all formats and codecs
	av_register_all();

	// Open video file
	if(avformat_open_input(&pFormatCtx, p_video_filename, NULL, NULL)!=0)
		return -1; // Couldn't open file

	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, p_video_filename, 0);

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
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
	fprintf(stderr, "Unsupported codec!\n");
	return -1; // Codec not found
    }
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
	return -1; // Could not open codec
  
    // Allocate video frame
    pFrame=avcodec_alloc_frame();
  	if(pFrame == NULL)
  		return -1;

  	// Allocate an AVFrame structure
    pFrameRGBResized=avcodec_alloc_frame();
    if(pFrameRGBResized==NULL)
	return -1;
	
	int targetWidth = 540;
	int targetHeight = 960;

    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(PIX_FMT_RGB24, /*pCodecCtx->width, pCodecCtx->height*/targetWidth, targetHeight);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    
    sws_ctx =
	sws_getContext
	(
	 pCodecCtx->width,
	 pCodecCtx->height,
	 pCodecCtx->pix_fmt,
	 targetWidth,
	 targetHeight,
	 //pCodecCtx->width,
	 //pCodecCtx->height,
	 PIX_FMT_RGB24,
	 SWS_BICUBIC,//SWS_BILINEAR,
	 NULL,
	 NULL,
	 NULL
	 );
    

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGBResized, buffer, PIX_FMT_RGB24,
		   /*pCodecCtx->width, pCodecCtx->height*/targetWidth, targetHeight);

    // Read frames and save first five frames to disk
    i=0;
    while(av_read_frame(pFormatCtx, &packet)>=0) {
	// Is this a packet from the video stream?
	if(packet.stream_index==videoStream) {
	    // Decode video frame
	    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				  &packet);
      
	    // Did we get a video frame?
	    if(frameFinished) {
		// Convert the image from its native format to RGB
		sws_scale
		    (
		     sws_ctx,
		     (uint8_t const * const *)pFrame->data,
		     pFrame->linesize,
		     0,
		     pCodecCtx->height,
		     //pFrameRGB->data,
		     //pFrameRGB->linesize,
		     pFrameRGBResized->data,
		     pFrameRGBResized->linesize
		     );

		printf("Read frame\n");
		// Save the frame to disk
		if(++i<=5) {
			*p_frame_image_data = pFrameRGBResized->data[0];//outPacket.data;
			//*pFrameRGBResized->data = NULL;
			*p_width = targetWidth; //pCodecCtx->width;
			*p_height = targetHeight; //pCodecCtx->height;
			*p_linesize = pFrameRGBResized->linesize[0];
			break;
		}

		else
		    break;
	    }
	}
    
	// Free the packet that was allocated by av_read_frame
	av_free_packet(&packet);
    }

    // Free the RGB image
    //av_free(buffer);
    av_free(pFrameRGBResized);
  
    // Free the YUV frame
    av_free(pFrame);
  
    // Close the codec
    avcodec_close(pCodecCtx);
  
    // Close the video file
    avformat_close_input(&pFormatCtx);

     sws_freeContext(sws_ctx);
	return 0;
}
