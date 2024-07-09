#pragma once
#include "./MediaSink.h"
#include "../RenderAPI.h"


namespace zrtsp2
{
	class UnityTextureSink : public MediaSink
	{
		private:
			const char *m_id;
			void* m_pTextureHandle;

			uint8_t* m_BufferTextureRGB;
	
			//todo : initialize current api and unityGfxRenderer

		public:
			UnityTextureSink(void* textureHandle, const char* id, int height, int width);
			~UnityTextureSink();

			int WriteAudio(AVFrame* audio_frame);
			int WriteVideo(RenderAPI* render_api,AVFrame* Video_frame);

			bool RenderEvent(RenderAPI* render_api);

		private:
	};
}

