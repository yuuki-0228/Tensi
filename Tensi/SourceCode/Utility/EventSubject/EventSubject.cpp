#include "EventSubject.h"
#ifdef ENABLE_OBSERVER

EventSubject::EventSubject()
	: m_EventGameClear()
{
	m_EventGameClear = std::make_unique<CSubject<EventGameClearParam>>();
}

EventSubject::~EventSubject()
{

}

//---------------------------.
// インスタンスの取得.
//---------------------------.
EventSubject* EventSubject::GetInstance()
{
	static std::unique_ptr<EventSubject> pInstance = std::make_unique<EventSubject>();
	return pInstance.get();
}

#endif