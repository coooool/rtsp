#include <string>
#include <vector>
#include "FFMpegTasksManager.h"
#include "FFMpegRTSPStream.h"
#include "UnityTextureSink.h"
#include <excpt.h>
#include <mutex>
#include <functional>
#include<thread>

using zrtsp2::FFMpegTasksManager;
using std::vector;
using zrtsp2::FFMpegRTSPStream;

bool _initEngine = false;
FFMpegTasksManager FFMpegTasksManager::m_instance = FFMpegTasksManager();
std::vector<FFMpegRTSPStream*>* streams = new std::vector<FFMpegRTSPStream*>();

std::mutex mtx;

void Lock() {
	std::lock_guard<std::mutex> lock(mtx);
}

FFMpegTasksManager::FFMpegTasksManager()
{
	m_PlayerSuccessFunc = NULL;
	m_PlayerErrorFunc = NULL;
	m_PreConctionFunc = NULL;
}

FFMpegTasksManager::~FFMpegTasksManager()
{
}

unsigned int zrtsp2::FFMpegTasksManager::GetTaskCount()
{
	return 0;
}


void zrtsp2::FFMpegTasksManager::CreatePlayerTask(const char* id, const char* url, void* texture_handle, unsigned int width, unsigned int height)
{
	Lock();

	// 存在相同的视频ID则不再创建
	if (this->ExistsTaskFromVid(id))
	{
		return;
	}

	this->TriggerPreConction(id);

	auto newTask = new FFMpegRTSPStream(id, url);

	if (newTask->InitStream() != 0) {
		// 初始化失败，无效的链接？
		this->TriggerPlayError(id);
	}
	else {
		auto sink = new zrtsp2::UnityTextureSink(texture_handle, id, height, width);
		newTask->setMediaSink(sink);
		streams->push_back(newTask);

		this->TriggerPlaySuccess(id);

		// 创建新的后台线程开始读取数据
		std::thread(std::bind(&FFMpegRTSPStream::ReadFrame, newTask, &mtx)).detach();
	}
}

// 是否存在指定的ID
bool zrtsp2::FFMpegTasksManager::ExistsTaskFromVid(const char* vid)
{
	for (auto it = streams->begin(); it != streams->end(); ++it) 
	{
		if (*it != NULL) {
			if ((*it)->NameIs(vid))
				return true;
		}
	}

	return false;
}

void zrtsp2::FFMpegTasksManager::ReleaseAllTask()
{
	streams->erase(streams->begin(), streams->end());
	streams->shrink_to_fit();
}

void zrtsp2::FFMpegTasksManager::ReleaseTask(const char* vid)
{
	for (auto it = streams->begin(); it != streams->end(); ++it)
	{
		auto item = (*it);

		if (item != NULL)
		{
			if (item->NameIs(vid)) {
				item->m_StopLoopRead = true;
				break;
			}
		}
	}
}

FFMpegTasksManager* zrtsp2::FFMpegTasksManager::Instance()
{
	return &m_instance;
}

void zrtsp2::FFMpegTasksManager::InitEngine(RenderAPI* render)
{
	if (_initEngine) 
		return;

	_initEngine = true;
	m_pRenderApi = render;

	avformat_network_init();
}

void zrtsp2::FFMpegTasksManager::LoopUpdateFrame()
{
	for (int i = streams->size() - 1; i >= 0; i--)
	{
		auto item = streams->at(i);

		if (item != NULL)
		{
			if (item->m_IdleDel) {
				auto index = streams->begin() + i;
				streams->erase(index);
			}
			else if (item->m_StopLoopRead == false)
				item->RenderEvent(getRenderApi());
		}
	}
}

void zrtsp2::FFMpegTasksManager::TriggerPlaySuccess(const char* vid)
{
	if (m_PlayerSuccessFunc != NULL) {
		m_PlayerSuccessFunc(vid);
	}
}

void zrtsp2::FFMpegTasksManager::TriggerPlayError(const char* vid)
{
	if (m_PlayerErrorFunc != NULL) {
		m_PlayerErrorFunc(vid);
	}
}

void zrtsp2::FFMpegTasksManager::TriggerPreConction(const char* vid)
{
	if (m_PreConctionFunc != NULL) {
		m_PreConctionFunc(vid);
	}
}
