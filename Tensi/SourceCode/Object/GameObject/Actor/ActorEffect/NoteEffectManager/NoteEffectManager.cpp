#include "NoteEffectManager.h"
#include "NoteEffect\NoteEffect.h"

namespace {
	constexpr float COOL_TIME = 0.3f;	// クールタイム.
}

CNoteEffectManager::CNoteEffectManager()
	: m_pNoteEffectList	()
	, m_CoolTime		( INIT_FLOAT )
	, m_EffectSize		( INIT_FLOAT )
	, m_IsAnimation		( false )
	, m_IsPause			( false )
{
}

CNoteEffectManager::~CNoteEffectManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CNoteEffectManager::Init()
{
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CNoteEffectManager::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;

	NoteEffectPlay();		// 音符エフェクトの再生.
	NoteEffectListDelete();	// 音符エフェクトリストの削除.

	for ( auto& n : m_pNoteEffectList ) n->Update( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void CNoteEffectManager::Render()
{
	for ( auto& n : m_pNoteEffectList ) n->Render();
}

//---------------------------.
// アニメーションの開始.
//---------------------------.
void CNoteEffectManager::Play( const float Size )
{
	// 眠りエフェクトアニメーションの開始.
	m_IsAnimation	= true;
	m_EffectSize	= Size;
}

//---------------------------.
// 一時停止.
//---------------------------.
void CNoteEffectManager::Pause( const bool Flag )
{
	// 一時停止の設定.
	m_IsPause = Flag;
}

//---------------------------.
// アニメーションの停止.
//---------------------------.
void CNoteEffectManager::Stop()
{
	// 眠りエフェクトアニメーションの停止.
	m_IsAnimation = false;
}

//---------------------------.
// 音符エフェクトの再生.
//---------------------------.
void CNoteEffectManager::NoteEffectPlay()
{
	if ( m_IsPause				) return;
	if ( m_IsAnimation == false ) return;

	// クールタイムを減らす.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// クールタイムの設定.
	m_CoolTime = COOL_TIME;

	// 使用していないのがあれば使用する.
	for ( auto& n : m_pNoteEffectList ) {
		if ( n->GetIsAnimPlay() ) continue;

		// アニメーションを開始させる.
		n->Play( m_Transform.Position, m_EffectSize );
		return;
	}

	// 新しく作成.
	m_pNoteEffectList.emplace_back( std::make_unique<CNoteEffect>() );

	// アニメーションを開始させる.
	m_pNoteEffectList.back()->Init();
	m_pNoteEffectList.back()->Play( m_Transform.Position, m_EffectSize );
}

//---------------------------.
// 音符エフェクトリストの削除.
//---------------------------.
void CNoteEffectManager::NoteEffectListDelete()
{
	for ( int i = static_cast< int >( m_pNoteEffectList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pNoteEffectList[i]->GetIsAnimPlay() ) break;
		m_pNoteEffectList.pop_back();
	}
}
