#include "UnityTextureSink.h"
#include "FFMpegTasksManager.h"
#include <libavcodec/avcodec.h>
#include <excpt.h>
#include <thread>
#include <mutex>

std::mutex render_mtx;

zrtsp2::UnityTextureSink::UnityTextureSink(void* textureHandle, const char * id, int height, int width)
	: MediaSink(height,width, AV_PIX_FMT_RGBA)
{
	//AV_PIX_FMT_ARGB != textureformat ARGB32
	//AV_PIX_FMT_RGBA ?= textureformat RGBA32
	m_id = id;
	m_pTextureHandle = textureHandle;

	// ∑÷≈‰ª∫≥Â«¯
	m_BufferTextureRGB =  new uint8_t[width * height * 4];
	memset(m_pBuffer, 0, width * height * 4);
}

zrtsp2::UnityTextureSink::~UnityTextureSink() 
{
	delete m_BufferTextureRGB;
	m_BufferTextureRGB = NULL;
}

int zrtsp2::UnityTextureSink::WriteAudio(AVFrame* audio_frame)
{
	return 0;
}

int zrtsp2::UnityTextureSink::WriteVideo(RenderAPI* render_api, AVFrame * Video_frame)
{
	int test = 0;

	if (!m_pTextureHandle)
		return -2;

	render_mtx.lock();

	//TODO adjust time here
	// Convert the image from its native format to RGB
	sws_scale(
		m_pSwsContext,
		(const uint8_t* const*)Video_frame->data,
		Video_frame->linesize,
		0, 
		Video_frame->height,
		m_pFrameDst->data, 
		m_pFrameDst->linesize);

	/*
	int textureRowPitch;
	void* textureDataPtr = (uint8_t*)render_api->BeginModifyTexture(m_pTextureHandle, m_Width, m_Height, &textureRowPitch);
	if (!textureDataPtr)
		return -1;

	unsigned char* dst = (unsigned char*)textureDataPtr;
	unsigned char* src = (unsigned char*)m_pFrameDst->data[0];

	memcpy(dst, src, m_Height * m_Width * 4);

	render_api->EndModifyTexture(m_pTextureHandle, m_Width, m_Height, textureRowPitch, textureDataPtr );
	*/

	unsigned char* src = (unsigned char*)m_pFrameDst->data[0];
	memcpy(m_BufferTextureRGB, src, m_Height * m_Width * 4);

	render_mtx.unlock();

	return test;

}
#include <utility>
bool zrtsp2::UnityTextureSink::RenderEvent(RenderAPI* render_api)
{
	render_mtx.lock();

	__try
	{
		int textureRowPitch;

		void* textureDataPtr = (uint8_t*)render_api->BeginModifyTexture(m_pTextureHandle, m_Width, m_Height, &textureRowPitch);
		if (!textureDataPtr)
		{
			render_mtx.unlock();
			return false;
		}

		unsigned char* dst = (unsigned char*)textureDataPtr;
		unsigned char* src = (unsigned char*)m_BufferTextureRGB;

		memcpy(dst, m_BufferTextureRGB, m_Height * m_Width * 4);

		render_api->EndModifyTexture(m_pTextureHandle, m_Width, m_Height, textureRowPitch, textureDataPtr);

		render_mtx.unlock();

		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		fprintf(stderr, "‰÷»æ ß∞‹...");
	}

	render_mtx.unlock();

	return false;
}

