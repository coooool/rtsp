#pragma once

#include "../Unity/IUnityGraphics.h"
#include "../RenderAPI.h"

#include "ffmpegenv.h"

namespace zrtsp2
{

	typedef void (*VideoCallBack)(const char* vid);

	// ����൱��RTSPPluginSingleton
	class FFMpegTasksManager
	{
		public:
			FFMpegTasksManager();
			~FFMpegTasksManager();

			unsigned int GetTaskCount();

			// ������������
			void CreatePlayerTask(
				const char* id, 
				const char* url, 
				void* texture_handle,
				unsigned int width, 
				unsigned int height
			);

			// �Ƿ������ƵID
			bool ExistsTaskFromVid(const char* vid);

			// �ͷ���������
			void ReleaseAllTask();

			void ReleaseTask(const char* vid);

		public:
			static FFMpegTasksManager* Instance();
			
			// ��Unity�ڵ��ַ������浽C++��
			static char* MakeStringCopy(const char* string) {
				if (string == NULL) return NULL;
				auto length = strlen(string) + 1;
				char* res = new char[length];
				strcpy_s(res, length, string);
				return res;
			}

			// Unity���ز��ʱ�������������ʼ�����
			void InitEngine(RenderAPI* _render);

			RenderAPI* getRenderApi() { return m_pRenderApi; }

			void LoopUpdateFrame();

			void BindPlaySuccessFunc(VideoCallBack func) {
				m_PlayerSuccessFunc = func;
			}

			void BindPlayErrorFunc(VideoCallBack func) {
				m_PlayerErrorFunc = func;
			}

			void BindPreConctionFunc(VideoCallBack func) {
				m_PreConctionFunc = func;
			}

			void TriggerPlaySuccess(const char* vid);
			void TriggerPlayError(const char* vid);
			void TriggerPreConction(const char* vid);

		private:
			static FFMpegTasksManager m_instance;

		private:
			RenderAPI* m_pRenderApi = NULL;

			VideoCallBack m_PlayerSuccessFunc;
			VideoCallBack m_PlayerErrorFunc;
			VideoCallBack m_PreConctionFunc;
	};

}