#pragma once
#include "FFMpegStream.h"
#include "../RenderAPI.h"

namespace zrtsp2
{

	class FFMpegRTSPStream : public FFMpegStream
	{
		public:
			FFMpegRTSPStream(const char* vid, const char* uri);
			~FFMpegRTSPStream();
		
			int InitStream();
			int CloseStream();	

			bool NameIs(const char* name);

			void RenderEvent(RenderAPI* render_api);

		private:
			const char* m_Vid;
	};

}