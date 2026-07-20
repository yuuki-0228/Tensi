#include "ExploreManager.h"
#include "..\..\..\..\ActorEffect\IconSmokeEffectManager\IconSmokeEffectManager.h"
#include "..\..\..\..\..\..\..\Common\Sprite\Sprite.h"
#include "..\..\..\..\..\..\..\Common\Font\Font.h"
#include "..\..\..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\..\..\Resource\FontResource\FontResource.h"
#include "..\..\..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\..\..\Utility\WindowsMessageBox\WindowsMessageBox.h"
#include "..\..\..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"
#include "..\..\..\..\..\..\..\Utility\Const\Const.h"
#include "..\..\..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\..\..\Utility\Color\Color.h"
#include "..\..\..\..\..\..\..\Utility\Log\Log.h"
#include "..\..\..\..\..\..\..\Utility\ThreadManager\ThreadManager.h"
#include <ctime>
#include <cstring>
#include <algorithm>

namespace {
	// スプライト名.
	const std::string SLIME_SMOKE_NAME	= "IconSmoke";		// 煙の画像名.
	const std::string GAUGE_NAME		= "IconSmokeGauge";	// ゲージの画像名.
	const std::string FONT_NAME			= "コーポレート・ロゴ ver2 Medium";	// フォント名.

	// メッセージボックスの文言.
	const std::string MSG_CAPTION		= "冒険";
	const std::string MSG_ASK			= "スライムを冒険に行かせますか？";
	const std::string MSG_LOST_FOLDER	= "探索中のフォルダーがなくなったため探索を終了しました";

	// 1MBのバイト数.
	const double MEGA_BYTE = 1024.0 * 1024.0;

	// 時間の単位( 秒 ).
	const long long SEC_PER_HOUR	= 3600;	// 1時間の秒数.
	const long long SEC_PER_MINUTE	= 60;	// 1分の秒数.

	// 座標が矩形の中にあるか.
	bool IsPointInRect( const D3DXPOSITION3& Pos, const RECT& Rect )
	{
		return  Pos.x >= static_cast<float>( Rect.left  ) && Pos.x <= static_cast<float>( Rect.right  ) &&
				Pos.y >= static_cast<float>( Rect.top   ) && Pos.y <= static_cast<float>( Rect.bottom );
	}

	// デスクトップアイコンの矩形をゲーム座標系に変換する.
	//	WindowManager が返すアイコン矩形はデスクトップ( SysListView32 )の
	//	ウィンドウ原点が画面(0,0)である前提で補正されているため、
	//	マルチモニタ等で原点がずれている場合はデスクトップ原点分だけ補正する.
	RECT ToGameRect( const RECT& Rect )
	{
		RECT Desk = {};
		GetWindowRect( WindowManager::GetDesktop(), &Desk );

		RECT Out = Rect;
		Out.left	+= Desk.left;
		Out.right	+= Desk.left;
		Out.top		+= Desk.top;
		Out.bottom	+= Desk.top;
		return Out;
	}

	// スムーズな補間値( 0.0～1.0 )を返す.
	float SmoothStep( const float t )
	{
		const float x = std::clamp( t, 0.0f, 1.0f );
		return x * x * ( 3.0f - 2.0f * x );
	}

	// スライムの向きの画像番号( Slime画像の横のパターン番号 ).
	const int SLIME_IMG_NORMAL	= 0;	// 正面.
	const int SLIME_IMG_RIGHT	= 1;	// 右向き.
	const int SLIME_IMG_LEFT	= 2;	// 左向き.
}

CExploreManager::CExploreManager()
	: m_pSlimeSprite	( nullptr )
	, m_pSlimeState		( nullptr )
	, m_pSmoke			( nullptr )
	, m_pGaugeSprite	( nullptr )
	, m_GaugeBackState	()
	, m_GaugeState		()
	, m_pFont			( nullptr )
	, m_FontState		()
	, m_SaveData		()
	, m_FolderPath		()
	, m_State			( enState::None )
	, m_IconClickRect	()
	, m_IconDrawRect	()
	, m_IconIndex		( 0 )
	, m_StartPos		( INIT_FLOAT3 )
	, m_TargetCenter	( INIT_FLOAT3 )
	, m_StartScale		( 1.0f )
	, m_TargetScale		( 1.0f )
	, m_Timer			( INIT_FLOAT )
	, m_PeekCoolTime	( INIT_FLOAT )
	, m_PeekTime		( INIT_FLOAT )
	, m_IconPollCoolTime( INIT_FLOAT )
	, m_SlimeDispW		( 1.0f )
	, m_SlimeDispH		( 1.0f )
	, m_SlimeRefSize	( 1.0f )
	, m_SmokeDispW		( 1.0f )
	, m_GaugeDispW		( 1.0f )
	, m_GaugeDispH		( 1.0f )
	, m_FontDispH		( 1.0f )
	, m_IsPeeking		( false )
	, m_IsAskShown		( false )
	, m_AskAnswer		( 0 )
{
}

CExploreManager::~CExploreManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
void CExploreManager::Init( CSprite* pSlimeSprite, SSpriteRenderState* pSlimeState, const float SlimeRefSize )
{
	m_pSlimeSprite	= pSlimeSprite;
	m_pSlimeState	= pSlimeState;
	m_SlimeRefSize	= SlimeRefSize;

	// スライム画像の大きさを取得.
	if ( m_pSlimeSprite != nullptr ) {
		m_SlimeDispW = m_pSlimeSprite->GetSpriteState().Disp.w;
		m_SlimeDispH = m_pSlimeSprite->GetSpriteState().Disp.h;
	}

	// 煙エフェクトの初期化.
	m_pSmoke = std::make_unique<CIconSmokeEffectManager>();
	m_pSmoke->Init();

	// 煙画像の大きさを取得.
	CSprite* pSmoke = SpriteResource::GetSprite( SLIME_SMOKE_NAME );
	if ( pSmoke != nullptr ) m_SmokeDispW = pSmoke->GetSpriteState().Disp.w;

	// ゲージ画像の取得.
	m_pGaugeSprite	= SpriteResource::GetSprite( GAUGE_NAME, &m_GaugeBackState );
	m_GaugeState	= m_GaugeBackState;
	if ( m_pGaugeSprite != nullptr ) {
		m_GaugeDispW = m_pGaugeSprite->GetSpriteState().Disp.w;
		m_GaugeDispH = m_pGaugeSprite->GetSpriteState().Disp.h;
	}
	m_GaugeBackState.Color	= Color4::White;		// 背景は白.
	m_GaugeState.Color		= Color4::Limegreen;	// ゲージは緑.

	// フォントの取得.
	m_pFont = FontResource::GetFont( FONT_NAME, &m_FontState );
	if ( m_pFont != nullptr ) m_FontDispH = m_pFont->GetFontState().Disp.h;
	m_FontState.TextAlign		= ETextAlign::Center;
	m_FontState.Color			= Color4::White;

	// セーブデータの登録.
	SaveDataManager::SetSaveData()->ExploreData = &m_SaveData;
	SaveDataManager::PushLoadFunction( [this]() { OnLoaded(); } );
}

//---------------------------.
// 更新.
//---------------------------.
void CExploreManager::Update( const float& DeltaTime )
{
	switch ( m_State ) {
	case enState::ShrinkIn:		ShrinkInUpdate( DeltaTime );	break;
	case enState::Ask:			AskUpdate();					break;
	case enState::Exploring:	ExploringUpdate( DeltaTime );	break;
	case enState::ExpandOut:	ExpandOutUpdate( DeltaTime );	break;
	default:													break;
	}

	// 煙は常に更新する( 発生済みの煙を消えるまで残すため ).
	m_pSmoke->Update( DeltaTime );
}

//---------------------------.
// サブウィンドウ( アイコンの後ろ )に描画.
//---------------------------.
void CExploreManager::SubRender()
{
	// 煙の描画.
	m_pSmoke->SubRender();

	// 探索中のみゲージと残り時間を描画.
	if ( m_State == enState::Exploring ) GaugeSubRender();
}

//---------------------------.
// 探索を開始できるか確認し、できるなら開始する.
//---------------------------.
bool CExploreManager::TryStart( const D3DXPOSITION3& SlimeCenter )
{
	if ( m_State != enState::None ) return false;

	// フォルダのアイコンの上で離したか調べる.
	const WindowManager::IconList List = WindowManager::GetDesktopIconList();
	for ( const auto& Icon : List ) {
		if ( Icon.Type != WindowManager::enDesktopIconType::Folder ) continue;

		// アイコン矩形をゲーム座標系に変換して判定する.
		const RECT ClickRect = ToGameRect( Icon.ClickRect );
		if ( IsPointInRect( SlimeCenter, ClickRect ) == false ) continue;

		// 対象フォルダを決定.
		m_IconIndex		= Icon.Index;
		m_IconClickRect	= ClickRect;
		m_IconDrawRect	= ToGameRect( Icon.DrawRect );
		m_FolderPath	= WindowManager::GetDesktopIconPath( Icon.Index );
		UpdateTarget();

		// 縮小の開始情報を保存.
		m_StartPos		= m_pSlimeState->Transform.Position;
		m_StartScale	= m_pSlimeState->Transform.Scale.x;
		m_Timer			= INIT_FLOAT;

		// 描画範囲をリセットする.
		m_pSlimeState->RenderArea = { 0.0f, 0.0f, -1.0f, -1.0f };

		m_State = enState::ShrinkIn;
		return true;
	}
	return false;
}

//---------------------------.
// スライムをサブウィンドウに描画するフェーズか.
//---------------------------.
bool CExploreManager::IsSlimeInSub() const
{
	switch ( m_State ) {
	case enState::Ask:			return true;
	case enState::Exploring:	return m_IsPeeking;
	case enState::ExpandOut:	return m_Timer < Const::Explore().EXPAND_TIME * 0.5f;
	default:					return false;
	}
}

//---------------------------.
// アイコンに縮小して入っていく更新.
//---------------------------.
void CExploreManager::ShrinkInUpdate( const float& DeltaTime )
{
	// フォルダが消えたら飛び出して中止.
	if ( LocateTargetIcon() == false ) {
		StartExpandOut();
		return;
	}
	UpdateTarget();

	m_Timer += DeltaTime;
	const float e = SmoothStep( m_Timer / Const::Explore().SHRINK_TIME );

	// 位置とスケールを補間する.
	m_pSlimeState->Transform.Position	= m_StartPos + ( m_TargetCenter - m_StartPos ) * e;
	const float Scale = m_StartScale + ( m_TargetScale - m_StartScale ) * e;
	m_pSlimeState->Transform.Scale		= { Scale, Scale, Scale };

	// アイコンサイズになったら確認へ.
	if ( m_Timer >= Const::Explore().SHRINK_TIME ) {
		m_pSlimeState->Transform.Position	= m_TargetCenter;
		m_pSlimeState->Transform.Scale		= { m_TargetScale, m_TargetScale, m_TargetScale };
		// 確認ダイアログの状態をリセットする.
		m_IsAskShown = false;
		m_AskAnswer  = 0;
		m_State = enState::Ask;
	}
}

//---------------------------.
// 冒険に行かせるか確認する.
//---------------------------.
void CExploreManager::AskUpdate()
{
	// 初回のみ確認ダイアログを表示する( 非ブロッキング表示なのでアプリは止まらない ).
	if ( m_IsAskShown == false ) {
		m_IsAskShown = true;
		m_AskAnswer  = 0;
		// 応答はワーカースレッドから書き込まれるため、結果をフラグに控えるだけにする.
		WindowsMessageBox::YesNoAsync( MSG_ASK, MSG_CAPTION, [this]( bool IsYes ) {
			m_AskAnswer = IsYes ? 1 : 2;
		} );
		return;
	}

	// 回答待ち( 0=未回答 / 1=はい / 2=いいえ ).
	const int Answer = m_AskAnswer.load();
	if ( Answer == 0 ) return;

	m_IsAskShown = false;
	if ( Answer == 1 ) {
		// はい → 探索開始.
		StartExplore();
	}
	else {
		// いいえ → 飛び出す.
		StartExpandOut();
	}
}

//---------------------------.
// 探索中の更新.
//---------------------------.
void CExploreManager::ExploringUpdate( const float& DeltaTime )
{
	// デスクトップのアイコン情報の再取得は負荷が高い( 他プロセスとの通信を伴う )ため,
	// 毎フレームではなく一定間隔でのみ, 追跡中のフォルダ1つ分だけ再取得する.
	m_IconPollCoolTime -= DeltaTime;
	if ( m_IconPollCoolTime <= 0.0f ) {
		m_IconPollCoolTime = Const::Explore().ICON_POLL_INTERVAL;

		// フォルダが消えたら中止する.
		if ( LocateTargetIcon() == false ) {
			WindowsMessageBox::InfoAsync( MSG_LOST_FOLDER, MSG_CAPTION );
			ClearSaveData();
			StartExpandOut();
			return;
		}
		UpdateTarget();
	}

	// 完了したか.
	const long long Now = static_cast<long long>( std::time( nullptr ) );
	if ( Now >= m_SaveData.EndTime ) {
		OnExploreComplete();
		ClearSaveData();
		StartExpandOut();
		return;
	}

	// 頭出しの処理.
	if ( m_IsPeeking ) {
		m_PeekTime += DeltaTime;

		// 各フェーズの時間.
		const float MaxRise	= static_cast<float>( m_IconDrawRect.bottom - m_IconDrawRect.top ) * Const::Explore().PEEK_RISE_RATE;
		const float RiseT	= Const::Explore().PEEK_RISE_TIME;
		const float WaitT	= Const::Explore().PEEK_WAIT_TIME;
		const float LookT	= Const::Explore().PEEK_LOOK_TIME;
		const int	Looks	= Const::Explore().PEEK_LOOK_COUNT;
		const float SinkT	= Const::Explore().PEEK_SINK_TIME;

		// フェーズの区切り( 上に出る→待機→きょろきょろ→潜る ).
		const float T1 = RiseT;
		const float T2 = T1 + WaitT;
		const float T3 = T2 + LookT * Looks;
		const float T4 = T3 + SinkT;

		const float t		= m_PeekTime;
		float		Rise	= MaxRise;
		int			Facing	= SLIME_IMG_NORMAL;
		if ( t < T1 ) {
			// 上に移動して頭を出す.
			Rise = ( RiseT > 0.0f ) ? MaxRise * ( t / RiseT ) : MaxRise;
		}
		else if ( t < T2 ) {
			// 少し待機する.
			Rise = MaxRise;
		}
		else if ( t < T3 ) {
			// 左右をきょろきょろする.
			Rise = MaxRise;
			const int Index = static_cast<int>( ( t - T2 ) / LookT );
			Facing = ( Index % 2 == 0 ) ? SLIME_IMG_LEFT : SLIME_IMG_RIGHT;
		}
		else if ( t < T4 ) {
			// 正面に戻って潜る.
			Rise = ( SinkT > 0.0f ) ? MaxRise * ( 1.0f - ( t - T3 ) / SinkT ) : 0.0f;
		}

		m_pSlimeState->IsDisp					= true;
		m_pSlimeState->Transform.Position		= m_TargetCenter;
		m_pSlimeState->Transform.Position.y	   -= Rise;
		m_pSlimeState->Transform.Scale			= { m_TargetScale, m_TargetScale, m_TargetScale };
		m_pSlimeState->AnimState.PatternNo.x	= Facing;

		// 一連の動きが終わったら潜って非表示にする.
		if ( t >= T4 ) {
			m_IsPeeking							= false;
			m_pSlimeState->IsDisp				= false;
			m_pSlimeState->AnimState.PatternNo.x = SLIME_IMG_NORMAL;
			m_pSmoke->Play();
			m_PeekCoolTime						= Random::GetRand( Const::Explore().PEEK_INTERVAL_MIN, Const::Explore().PEEK_INTERVAL_MAX );
		}
	}
	else {
		// 潜って非表示.
		m_pSlimeState->IsDisp = false;

		m_PeekCoolTime -= DeltaTime;
		if ( m_PeekCoolTime <= 0.0f ) {
			// 頭出しを開始する( 煙を止める ).
			m_IsPeeking	= true;
			m_PeekTime	= INIT_FLOAT;
			m_pSmoke->Stop();
		}
	}

	// 煙の位置・大きさ・ずれ幅を更新する( アイコンサイズ連動 ).
	m_pSmoke->SetPosition( m_TargetCenter );
	const float IconW = static_cast<float>( m_IconDrawRect.right - m_IconDrawRect.left );
	if ( m_SmokeDispW > 0.0f ) m_pSmoke->SetSize( IconW / m_SmokeDispW * Const::Explore().SMOKE_SIZE_RATE );
	m_pSmoke->SetSpread( IconW * Const::Explore().SMOKE_SPREAD_RATE );
}

//---------------------------.
// アイコンから拡大して飛び出す更新.
//---------------------------.
void CExploreManager::ExpandOutUpdate( const float& DeltaTime )
{
	m_pSlimeState->IsDisp = true;

	m_Timer += DeltaTime;
	const float e = SmoothStep( m_Timer / Const::Explore().EXPAND_TIME );

	// 少し上へ飛び出す.
	D3DXPOSITION3 EndPos = m_TargetCenter;
	EndPos.y -= m_SlimeDispH * 0.5f;

	m_pSlimeState->Transform.Position	= m_StartPos + ( EndPos - m_StartPos ) * e;
	const float Scale = m_StartScale + ( 1.0f - m_StartScale ) * e;
	m_pSlimeState->Transform.Scale		= { Scale, Scale, Scale };

	// 通常サイズに戻ったら終了.
	if ( m_Timer >= Const::Explore().EXPAND_TIME ) {
		m_pSlimeState->Transform.Scale = STransform::NORMAL_SCALE_VEC3;
		m_State = enState::None;
	}
}

//---------------------------.
// 探索を開始する.
//---------------------------.
void CExploreManager::StartExplore()
{
	// 完了時間は最短時間で仮置きして開始し、フォルダ容量の計算後に確定する.
	//	( フォルダ容量の計算は中身の再帰走査で重いことがあるため、ワーカースレッドで行う ).
	const double DurationSec = Const::Explore().COMPLETE_TIME_MIN;

	// セーブデータに探索情報を設定.
	m_SaveData.IsExploring	= true;
	m_SaveData.StartTime	= static_cast<long long>( std::time( nullptr ) );
	m_SaveData.EndTime		= m_SaveData.StartTime + static_cast<long long>( DurationSec );
	strncpy_s( m_SaveData.FolderPath, sizeof( m_SaveData.FolderPath ), m_FolderPath.c_str(), _TRUNCATE );

	// スライムを潜行させて非表示にする.
	m_pSlimeState->IsDisp = false;

	// 煙を開始する.
	m_pSmoke->SetPosition( m_TargetCenter );
	m_pSmoke->Play();

	// 頭出しの時間を初期化する.
	m_IsPeeking		= false;
	m_PeekCoolTime	= Random::GetRand( Const::Explore().PEEK_INTERVAL_MIN, Const::Explore().PEEK_INTERVAL_MAX );

	// アイコン情報の再取得間隔を初期化する.
	m_IconPollCoolTime = INIT_FLOAT;

	m_State = enState::Exploring;

	// すぐにセーブする.
	SaveDataManager::Save();

	// フォルダの容量をワーカースレッドで計算し、完了時間を確定する.
	if ( LocateTargetIcon() ) {
		const std::string SizePath = WindowManager::GetDesktopIconFilePath( m_IconIndex );
		if ( SizePath.empty() == false ) {
			const std::string FolderPath = m_FolderPath;
			ThreadManager::StartResult(
				[SizePath]() { return WindowManager::GetPathTotalSize( SizePath ); },
				[this, FolderPath]( const ULONGLONG& Bytes ) {
					// 計算中に探索が中断・別フォルダに変更された場合は反映しない.
					if ( m_SaveData.IsExploring == false )	return;
					if ( FolderPath != m_FolderPath )		return;

					// 容量から完了時間を求めてセーブデータへ反映する.
					const double MegaByte	= static_cast<double>( Bytes ) / MEGA_BYTE;
					const double NewSec		= std::clamp( MegaByte * Const::Explore().COMPLETE_SEC_PER_MB,
						static_cast<double>( Const::Explore().COMPLETE_TIME_MIN ),
						static_cast<double>( Const::Explore().COMPLETE_TIME_MAX ) );
					m_SaveData.EndTime = m_SaveData.StartTime + static_cast<long long>( NewSec );
					SaveDataManager::Save();
				} );
		}
	}
}

//---------------------------.
// アイコンから飛び出す演出を開始する.
//---------------------------.
void CExploreManager::StartExpandOut()
{
	m_pSmoke->Stop();

	// アイコンの位置から始める.
	m_StartPos		= m_TargetCenter;
	m_StartScale	= m_TargetScale;

	m_pSlimeState->IsDisp				= true;
	m_pSlimeState->Transform.Position	= m_TargetCenter;
	m_pSlimeState->Transform.Scale		= { m_TargetScale, m_TargetScale, m_TargetScale };

	m_Timer	= INIT_FLOAT;
	m_State	= enState::ExpandOut;
}

//---------------------------.
// 探索の情報を初期化してセーブデータを解除する.
//---------------------------.
void CExploreManager::ClearSaveData()
{
	m_SaveData.IsExploring	= false;
	m_SaveData.StartTime	= 0;
	m_SaveData.EndTime		= 0;
	m_SaveData.FolderPath[0] = '\0';

	// すぐにセーブする.
	SaveDataManager::Save();
}

//---------------------------.
// 進捗がMAXになった時の処理( 特定の処理 ).
//---------------------------.
void CExploreManager::OnExploreComplete()
{
	// TODO: 探索完了時の処理をここに実装する.
	Log::PushLogInfo( "探索完了 : " + m_FolderPath );
}

//---------------------------.
// 読み込み時の復帰処理.
//---------------------------.
void CExploreManager::OnLoaded()
{
	if ( m_SaveData.IsExploring == false ) return;

	m_FolderPath = m_SaveData.FolderPath;

	// 可能ならアイコンの位置を取得しておく.
	//	フォルダ消失・完了の判定は最初の探索中の更新で行う( デスクトップの準備待ちのため ).
	if ( LocateTargetIcon() ) UpdateTarget();

	// 探索を再開する.
	m_pSlimeState->IsDisp = false;
	m_pSmoke->SetPosition( m_TargetCenter );
	m_pSmoke->Play();
	m_IsPeeking			= false;
	m_PeekCoolTime		= Random::GetRand( Const::Explore().PEEK_INTERVAL_MIN, Const::Explore().PEEK_INTERVAL_MAX );
	m_IconPollCoolTime	= Const::Explore().ICON_POLL_INTERVAL;
	m_State				= enState::Exploring;
}

//---------------------------.
// 対象フォルダのアイコンを探し情報を更新する.
//---------------------------.
bool CExploreManager::LocateTargetIcon()
{
	const WindowManager::IconList List = WindowManager::GetDesktopIconList();
	for ( const auto& Icon : List ) {
		if ( Icon.Type != WindowManager::enDesktopIconType::Folder ) continue;
		if ( WindowManager::GetDesktopIconPath( Icon.Index ) != m_FolderPath ) continue;

		m_IconIndex		= Icon.Index;
		m_IconClickRect	= ToGameRect( Icon.ClickRect );
		m_IconDrawRect	= ToGameRect( Icon.DrawRect );
		return true;
	}
	return false;
}

//---------------------------.
// アイコンの矩形から目標位置・目標スケールを求める.
//---------------------------.
void CExploreManager::UpdateTarget()
{
	// アイコンの描画範囲の中心を目標位置とする.
	m_TargetCenter.x = static_cast<float>( m_IconDrawRect.left + m_IconDrawRect.right  ) * 0.5f;
	m_TargetCenter.y = static_cast<float>( m_IconDrawRect.top  + m_IconDrawRect.bottom ) * 0.5f;
	m_TargetCenter.z = 0.0f;

	// アイコンの幅に合わせたスケールを求める.
	//	画像の余白(透明部分)を含まない見た目基準サイズ( m_SlimeRefSize )を元に計算することで、
	//	実際に見えるスライムの大きさがアイコン幅の割合どおりになるようにする.
	const float IconW = static_cast<float>( m_IconDrawRect.right - m_IconDrawRect.left );
	m_TargetScale = ( m_SlimeRefSize > 0.0f ) ? ( IconW / m_SlimeRefSize * Const::Explore().SLIME_ICON_SCALE_RATE ) : 1.0f;
}

//---------------------------.
// ゲージと残り時間の描画.
//---------------------------.
void CExploreManager::GaugeSubRender()
{
	// アイコンの大きさを基準にする.
	const float IconH	= static_cast<float>( m_IconDrawRect.bottom - m_IconDrawRect.top );
	const float ClickW	= static_cast<float>( m_IconClickRect.right - m_IconClickRect.left );
	const float CenterX	= static_cast<float>( m_IconClickRect.left + m_IconClickRect.right ) * 0.5f;

	// ゲージの大きさ・位置( アイコンサイズ連動. フォルダ名の下あたり ).
	const float GaugeW	= ClickW * Const::Explore().GAUGE_WIDTH_RATE;
	const float GaugeH	= IconH  * Const::Explore().GAUGE_HEIGHT_RATE;
	const float LeftX	= CenterX - GaugeW * 0.5f;
	const float GaugeY	= static_cast<float>( m_IconClickRect.bottom ) + IconH * Const::Explore().GAUGE_OFFSET_Y_RATE;

	const float ScaleX	= ( m_GaugeDispW > 0.0f ) ? ( GaugeW / m_GaugeDispW ) : 1.0f;
	const float ScaleY	= ( m_GaugeDispH > 0.0f ) ? ( GaugeH / m_GaugeDispH ) : 1.0f;
	const float Progress = GetProgress();

	// 背景( 白 ).
	m_GaugeBackState.Transform.Position	= { LeftX, GaugeY, 0.0f };
	m_GaugeBackState.Transform.Scale	= { ScaleX, ScaleY, 1.0f };
	m_pGaugeSprite->RenderUI( &m_GaugeBackState );

	// ゲージ( 緑 ).
	m_GaugeState.Transform.Position		= { LeftX, GaugeY, 0.0f };
	m_GaugeState.Transform.Scale		= { ScaleX * Progress, ScaleY, 1.0f };
	m_pGaugeSprite->RenderUI( &m_GaugeState );

	// 残り時間の文字( アイコンサイズ連動 ).
	const float TextScale = ( m_FontDispH > 0.0f ) ? ( IconH * Const::Explore().TEXT_HEIGHT_RATE / m_FontDispH ) : 1.0f;
	m_FontState.Transform.Scale = { TextScale*1.5f, TextScale*1.5f, TextScale };

	const long long Now		= static_cast<long long>( std::time( nullptr ) );
	const long long Remain	= m_SaveData.EndTime - Now;
	m_FontState.Text				= MakeRemainText( Remain );
	m_FontState.Transform.Position	= { CenterX, GaugeY + GaugeH + IconH * Const::Explore().TEXT_OFFSET_Y_RATE, 0.0f };
	if ( m_pFont != nullptr ) m_pFont->RenderUI( &m_FontState );
}

//---------------------------.
// 残り時間の文字列を作成する.
//---------------------------.
std::string CExploreManager::MakeRemainText( const long long RemainSec ) const
{
	long long		Remain = RemainSec;
	if ( Remain < 0 ) Remain = 0;

	long long	Value = 0;
	std::string	Unit;
	if ( Remain >= SEC_PER_HOUR ) {
		// 1時間以上は時間表示.
		Value	= Remain / SEC_PER_HOUR;
		Unit	= "時間";
	}
	else if ( Remain >= SEC_PER_MINUTE ) {
		// 59分以下は分表示.
		Value	= Remain / SEC_PER_MINUTE;
		Unit	= "分";
	}
	else {
		// 59秒以下は秒表示.
		Value	= Remain;
		Unit	= "秒";
	}

	return "あと" + std::to_string( Value ) + Unit;
}

//---------------------------.
// 進捗率( 0.0～1.0 )を取得.
//---------------------------.
float CExploreManager::GetProgress() const
{
	const long long Total = m_SaveData.EndTime - m_SaveData.StartTime;
	if ( Total <= 0 ) return 1.0f;

	const long long Now		= static_cast<long long>( std::time( nullptr ) );
	const float		Progress = static_cast<float>( Now - m_SaveData.StartTime ) / static_cast<float>( Total );
	return std::clamp( Progress, 0.0f, 1.0f );
}
