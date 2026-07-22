#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_OBSERVER
#include "Subject/Subject.h"
#include "EventParam/EventParam.h"
#include <memory>

/************************************************
*	オブザーバークラス
**/
class EventSubject
{
public:
	EventSubject();
	~EventSubject();

	static CSubject<EventGameClearParam>* OnGameClear() { return GetInstance()->m_EventGameClear.get(); }

private:
	// インスタンスの取得.
	static EventSubject* GetInstance();

private:
	std::unique_ptr<CSubject<EventGameClearParam>> m_EventGameClear;

};

#endif