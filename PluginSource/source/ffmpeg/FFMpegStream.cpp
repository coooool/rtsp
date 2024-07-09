#include "FFMpegStream.h"
#include "FFMpegTasksManager.h"
#include <mutex>

zrtsp2::FFMpegStream::FFMpegStream(const char* uri)
{
	m_pStreamUri = uri;
	m_pMediaSink = NULL;
}


zrtsp2::FFMpegStream::~FFMpegStream()
{

}
/*
	return -1 if the stream as not been set yet.

*/
int zrtsp2::FFMpegStream::setMediaSink(MediaSink* media_sink)
{
	if (hasInit()){
		media_sink->setSrcCodecContext(m_pCodecCtx);
		m_pMediaSink = media_sink;
		return 0;
	}
	return -1;
}

const char * zrtsp2::FFMpegStream::getUri() const
{
	return m_pStreamUri;
}


/*
int rtsp_unity_plugin::FFMpegStream::addMediaSink(MediaSink &media_sink)
{
	media_sink.setSrcCodecContext(m_pCodecCtx);
	m_vMediaSink.push_back(media_sink);
	return 0;
}*/
/* will read frame and send them to the mediaSink
   if no frame are available, close the stream and return 0
   if frame not yet available, return 1
   else return 2; (2 mean ok!!)

*/
int zrtsp2::FFMpegStream::ReadFrame(std::mutex* mtx)
{
	if (m_hasInit == false)
		return -1;

	auto ffmpegClassPtr = FFMpegTasksManager::Instance();

	while (!m_StopLoopRead) 
	{
		if (av_read_frame(m_pFormatCtx, m_packet) >= 0) {
			
			if (m_StopLoopRead)
				break;

			if (m_packet->stream_index == m_VideoStreamIndex) {
				// Supply raw packet data as input to a decoder.
				avcodec_send_packet(m_pCodecCtx, m_packet);
				// Decode video frame
				m_frameFinished = avcodec_receive_frame(m_pCodecCtx, m_pFrameSrc);
				// Did we get a video complete video frame?
				if (m_frameFinished == 0) {
					if (m_pMediaSink != NULL)
						m_pMediaSink->WriteVideo(ffmpegClassPtr->getRenderApi(), m_pFrameSrc);
				}
			}
			else if (m_packet->stream_index == m_AudioStreamIndex) {
				// TODO Handle the audio output!!!
				if (m_pMediaSink != NULL)
					m_pMediaSink->WriteAudio(m_pFrameSrc);
			}

			av_packet_unref(m_packet);
		}
		else {
			break;
		}

		//std::this_thread::yield();
		//std::this_thread::sleep_for(std::chrono::milliseconds(400));
	}

	this->CloseStream();
	this->m_IdleDel = true;

	return 0;
}
