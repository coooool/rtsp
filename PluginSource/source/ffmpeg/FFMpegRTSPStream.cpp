#include "FFMpegRTSPStream.h"
#include "FFMpegTasksManager.h"
#include <string>
#include <excpt.h>
#include <thread>
#include <functional>
#include "UnityTextureSink.h"

zrtsp2::FFMpegRTSPStream::FFMpegRTSPStream(const char* vid, const char* uri) : FFMpegStream(uri)
{
	this->m_Vid = vid;
	this->m_isClosed = false;
	this->m_hasInit = false;
	this->m_StopLoopRead = false;
	this->m_IdleDel = false;
}


zrtsp2::FFMpegRTSPStream::~FFMpegRTSPStream()
{
	CloseStream();
}

int zrtsp2::FFMpegRTSPStream::InitStream()
{
	__try
	{
		AVIOContext* io_ctx = NULL;
		AVDictionary* open2_dict = NULL;

		//----------------------------------------------
		// �ȼ�������Ƿ���Ч
		// ���ó�ʱѡ��5��
		//av_dict_set(&open2_dict, "timeout", "5000000", 0);

		//if (avio_open2(&io_ctx, m_pStreamUri, AVIO_FLAG_READ, NULL, &open2_dict) < 0)
		//	return -1;
		

		// ---------------------------------------------
		// ����֡����
		// ----------------------------------------------
		m_pFormatCtx = avformat_alloc_context();
		if (m_pFormatCtx == NULL) {
			// ��������ʧ�� 
			return -2;
		}

		// ��������
		//m_pFormatCtx->pb = io_ctx;

		// ---------------------------------------------
		//�������ļ���ʹ��avformat_open_input()�����������ļ�������ȡ�ļ��ĸ�ʽ��Ϣ��
		// ---------------------------------------------
		AVDictionary* options = NULL;
		// RTSPָ��ΪTCP����
		av_dict_set(&options, "rtsp_transport", "tcp", 0);
		av_dict_set(&options, "timeout", "5000000", 0); // ���ó�ʱʱ��Ϊ5��
		av_dict_set(&options, "stimeout", "5000000", 0); // ���ó�ʱʱ��Ϊ5��

		if (avformat_open_input(&m_pFormatCtx, m_pStreamUri, NULL, &options) != 0)
		{
			m_IdleDel = true;
			avformat_free_context(m_pFormatCtx);
			return -3;
		}

		// ��ȫ��� 
		if (m_StopLoopRead)
			return -1;

		// ----------------------------------------------
		// ������������ļ���ȡ�ļ���ʽ��Ϣ
		// ----------------------------------------------
		AVDictionary* readinfo_options = NULL;
		av_dict_set(&readinfo_options, "probe_size ", "32", 0);

		if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0)
		{
			avformat_free_context(m_pFormatCtx);
			fprintf(stderr, "��ȡ��Ƶ����Ϣʧ��...");
			return -1;
		}

		// ��ȫ��� 
		if (m_StopLoopRead)
			return -1;

		// ----------------------------------------------
		// �ҵ���Ƶ����ʹ��av_find_best_stream()����������Ƶ����
		// ----------------------------------------------
		m_VideoStreamIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		if (m_VideoStreamIndex < 0) {
			// ������
			fprintf(stderr, "��ȡ��Ƶ����Ϣʧ��..");
			return -1;
		}

		m_AudioStreamIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
		if (m_AudioStreamIndex < 0) {
			// ������
			fprintf(stderr, "��ȡ��Ƶ����Ϣʧ��..");
			//return -1;
		}

		// ---------------------------------------------
		// ����Ƶ��������ʹ��avcodec_find_decoder()����������Ƶ����������ʹ��avcodec_open2()�����򿪽�������
		// ---------------------------------------------
		AVCodecParameters* pCodecPar = NULL;
		pCodecPar = m_pFormatCtx->streams[m_VideoStreamIndex]->codecpar;

		m_pCodec = avcodec_find_decoder(pCodecPar->codec_id);
		if (m_pCodec == NULL)
		{
			fprintf(stderr, "��֧�ֵı����ʽ!\n");
			return -1; 
		}

		// ���ñ��뻷�� (create new codec_context)
		m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
		if (avcodec_parameters_to_context(m_pCodecCtx, pCodecPar) < 0)
		{
			fprintf(stderr, "Couldn't copy codec context");
			return -1;
		}

		// Open codec
		if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0)
			return -1;

		// ��ȫ��� 
		if (m_StopLoopRead)
			return -1;

		m_packet = av_packet_alloc();
		if (NULL == m_packet)
			return -1;

		// Allocate video frame
		// Allocate an AVFrame structure
		m_pFrameSrc = av_frame_alloc();

		//play RTSP
		av_read_play(m_pFormatCtx);

		m_hasInit = true;
		m_isClosed = false;

		return 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		m_StopLoopRead = true;
		fprintf(stderr, "������ʧ��...");
	}

	m_StopLoopRead = true;

	return -1;
}

int zrtsp2::FFMpegRTSPStream::CloseStream()
{
	__try
	{
		if (!isClosed() && hasInit()) {
			m_isClosed = true;
			m_hasInit = false;

			m_StopLoopRead = true;
			//av_read_pause(m_pFormatCtx);

			// free the source frame
			av_free(m_pFrameSrc);
			// Close the codecs
			avcodec_close(m_pCodecCtx);

			avformat_close_input(&m_pFormatCtx);

			// free the format context
			if(m_pFormatCtx != NULL)
			avformat_free_context(m_pFormatCtx);
		
			if (m_pMediaSink != NULL) {
				delete m_pMediaSink;
				m_pMediaSink = NULL;
			}
			return 0;
		}
		else if (!hasInit()) {
			return 1; // 1= not yet initialized
		}
		else {
			return 2; // already closed;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		fprintf(stderr, "������ʧ��...");
	}

	m_IdleDel = true;

	return 0;
}

bool zrtsp2::FFMpegRTSPStream::NameIs(const char* name)
{
	return strcmp(this->m_Vid, name) == 0;
}

void zrtsp2::FFMpegRTSPStream::RenderEvent(RenderAPI* render_api)
{
	if (m_pMediaSink != NULL) {
		auto item = dynamic_cast<UnityTextureSink*>(m_pMediaSink);
		
		if (item != NULL)
		{
			if (!item->RenderEvent(render_api))
			{
				this->m_StopLoopRead = true;
			}
		}
	}
}
