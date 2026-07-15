#include "Widget.h"

CWidget::CWidget()
	: m_Wnd_W	( Const::GameWindow().SIZE.x )
	, m_Wnd_H	( Const::GameWindow().SIZE.y )
{
}

CWidget::~CWidget()
{
}
