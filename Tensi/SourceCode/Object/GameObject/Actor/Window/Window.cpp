#include "Window.h"
#include "..\..\..\..\Common\Sprite\Sprite.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\Resource\FontResource\FontResource.h"
#include "..\..\..\..\Utility\Input\Input.h"

namespace {
	constexpr float CANCEL_DISP_POS[3] = { -2.0f, 2.0f, 2.0f };	// 閉じるボタンの表示位置
}

CWindow::CWindow()
	: m_pSprite			( nullptr )
	, m_pCancelSprite	( nullptr )
	, m_pFont			( nullptr )
	, m_SpriteState		()
	, m_CancelState		()
	, m_FontState		()
	, m_SpriteData		()
	, m_CancelData		()
	, m_LeftUpPos		( INIT_FLOAT3 )
	, m_Size			( INIT_FLOAT2 )
	, m_DispAreaPos		( INIT_FLOAT4 )
	, m_DispAreaSize	( INIT_FLOAT2 )
	, m_IsDispCancel	( true )
{
}

CWindow::~CWindow()
{
}

//---------------------------.
// 初期化
//---------------------------.
bool CWindow::Init()
{
	m_pSprite		= SpriteResource::GetSprite( "Window",		&m_SpriteState );
	m_pCancelSprite = SpriteResource::GetSprite( "WindowClose",	&m_CancelState );
	m_pFont			= FontResource::GetFont( "コーポレート・ロゴ ver2 Medium", &m_FontState );
	m_SpriteState.AnimState.IsSetAnimNumber = true;

	m_SpriteState.MaskTexture = SpriteResource::GetSprite( "WindowMask" )->GetTexture();
	m_SpriteState.IsColorMask = true;

	m_SpriteData = m_pSprite->GetSpriteState();
	m_CancelData = m_pCancelSprite->GetSpriteState();
	return true;
}

//---------------------------.
// 更新
//---------------------------.
void CWindow::Update( const float& DeltaTime )
{
	if ( m_IsDispCancel ) {
		const auto& mPos	= Input::GetMousePosition();
		const auto* pPos	= &m_CancelState.Transform.Position;

		// マウスに重なっているか
		m_CancelState.Color = Color4::White;
		if ( pPos->x - m_CancelData.Disp.w <= mPos.x && mPos.x <= pPos->x &&
			 pPos->y <= mPos.y && mPos.y <= pPos->y + m_CancelData.Disp.h )
		{
			m_CancelState.Color = Color4::Red;

			// クリックされたか
			if ( Input::GetIsLeftClickDown() ) {
				m_IsDisp = false;
			}
		}
	}


	if ( KeyInput::IsKeyPress( VK_RIGHT ) ) {
		m_Size.x += 0.1f;
	}
	if ( KeyInput::IsKeyPress( VK_LEFT ) ) {
		m_Size.x -= 0.1f;
	}
	if ( KeyInput::IsKeyPress( VK_DOWN ) ) {
		m_Size.y += 0.1f;
	}
	if ( KeyInput::IsKeyPress( VK_UP ) ) {
		m_Size.y -= 0.1f;
	}
}

//---------------------------.
// 描画
//---------------------------.
void CWindow::SubRender()
{
	if ( m_IsDisp == false ) return;

	// 左上
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.AnimState.AnimNumber	= 0;
	m_pSprite->RenderUI( &m_SpriteState );

	// タイトルの位置の設定
	m_FontState.Transform.Position		= m_LeftUpPos;
	m_FontState.Transform.Position.y   += 2.0f;

	// 上
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Position.x += m_SpriteData.Disp.w;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.Transform.Scale.x		= m_Size.x;
	m_SpriteState.AnimState.AnimNumber	= 1;
	m_pSprite->RenderUI( &m_SpriteState );

	// 右上
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Position.x += m_SpriteData.Disp.w + m_SpriteData.Disp.w * m_Size.x;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.AnimState.AnimNumber	= 2;
	m_pSprite->RenderUI( &m_SpriteState );

	// 閉じるボタンの位置の設定
	m_CancelState.Transform.Position	= m_SpriteState.Transform.Position + D3DXPOSITION3( CANCEL_DISP_POS );
	m_CancelState.Transform.Position.x += m_SpriteData.Disp.w;

	// 左中央
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Position.y	= m_SpriteData.Disp.h;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.Transform.Scale.y		= m_Size.y;
	m_SpriteState.AnimState.AnimNumber	= 3;
	m_pSprite->RenderUI( &m_SpriteState );

	// 中央
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Position.x += m_SpriteData.Disp.w;
	m_SpriteState.Transform.Position.y	= m_SpriteData.Disp.h;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.Transform.Scale.x		= m_Size.x;
	m_SpriteState.Transform.Scale.y		= m_Size.y;
	m_SpriteState.AnimState.AnimNumber	= 4;
	m_pSprite->RenderUI( &m_SpriteState );

	// 右中央
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Position.x += m_SpriteData.Disp.w + m_SpriteData.Disp.w * m_Size.x;
	m_SpriteState.Transform.Position.y	= m_SpriteData.Disp.h;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.Transform.Scale.y		= m_Size.y;
	m_SpriteState.AnimState.AnimNumber	= 5;
	m_pSprite->RenderUI( &m_SpriteState );

	// 左下
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Position.y	= m_SpriteData.Disp.h + m_SpriteData.Disp.h * m_Size.y;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.AnimState.AnimNumber	= 6;
	m_pSprite->RenderUI( &m_SpriteState );

	// 下
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Position.x += m_SpriteData.Disp.w;
	m_SpriteState.Transform.Position.y	= m_SpriteData.Disp.h + m_SpriteData.Disp.h * m_Size.y;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.Transform.Scale.x		= m_Size.x;
	m_SpriteState.AnimState.AnimNumber	= 7;
	m_pSprite->RenderUI( &m_SpriteState );

	// 右上
	m_SpriteState.Transform.Position	= m_LeftUpPos;
	m_SpriteState.Transform.Position.x += m_SpriteData.Disp.w + m_SpriteData.Disp.w * m_Size.x;
	m_SpriteState.Transform.Position.y	= m_SpriteData.Disp.h + m_SpriteData.Disp.h * m_Size.y;
	m_SpriteState.Transform.Scale		= STransform::NORMAL_SCALE_VEC3;
	m_SpriteState.AnimState.AnimNumber	= 8;
	m_pSprite->RenderUI( &m_SpriteState );

	// タイトルの表示
	m_pFont->RenderUI( &m_FontState );

	// 閉じるボタン
	if ( m_IsDispCancel ) {
		m_pCancelSprite->RenderUI( &m_CancelState );
	}

	m_RenderFunction();
}

//---------------------------.
// 作成
//---------------------------.
void CWindow::Create( const D3DXPOSITION3& pos, const D3DXSCALE2& size, const std::string& title, const D3DXCOLOR4& wndColor, const bool dispCancel )
{
	m_Size			= size;
	m_LeftUpPos		= pos;
	m_IsDispCancel	= dispCancel;

	m_SpriteState.Color = wndColor;

	// タイトルの設定
	m_FontState.Transform.Position	 = m_LeftUpPos;
	m_FontState.Transform.Position.y += 2.0f;
	m_FontState.Color				 = Color4::Ghostwhite;
	m_FontState.Text				 = title;

	// 描画エリア情報の設定
	m_DispAreaSize.x = m_SpriteData.Disp.w * m_Size.x + 19.0f + 19.0f;
	m_DispAreaSize.y = m_SpriteData.Disp.h * m_Size.y +  1.0f + 19.0f;

	m_DispAreaPos.x	= m_LeftUpPos.x + 5.0f;
	m_DispAreaPos.y	= m_LeftUpPos.y + 23.0f;
	m_DispAreaPos.z = m_DispAreaPos.x + m_DispAreaSize.x;
	m_DispAreaPos.w = m_DispAreaPos.y + m_DispAreaSize.y;

	// 閉じるボタンの設定
	if( m_IsDispCancel ){
		m_CancelState.Transform.Position	 = m_LeftUpPos;
		m_CancelState.Transform.Position.x	+= m_SpriteData.Disp.w * 2 + m_SpriteData.Disp.w * m_Size.x;
		m_CancelState.Transform.Position	+= CANCEL_DISP_POS;
	}
}
