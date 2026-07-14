#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_OBSERVER
#include <vector>
#include <functional>


/************************************************
*	SubjectÉNÉâÉX
**/
template<typename  T>
class CSubject
{
public:
	CSubject()
		: m_ObserverList()
	{};
	~CSubject() {};

	//---------------------------.
	// åƒÇ—èoÇµ
	//---------------------------.
	void Call( const T& Param ) {
		for ( auto& observer : m_ObserverList ) {
			observer( Param );
		}
	}

	//---------------------------.
	// ObserverÇÃí«â¡
	//---------------------------.
	void Subscribe( std::function<void( T )> observer ) {
		m_ObserverList.emplace_back( observer );
	}

private:
	std::vector<std::function<void( T )>> m_ObserverList;
};

#endif