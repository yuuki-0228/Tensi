#include "SceneWidget.h"
#include "..\..\..\..\Utility\StringConversion\StringConversion.h"
#include "..\..\..\..\Utility\ImGuiManager\ImGuiManager.h"
#include "..\..\..\..\Utility\Input\Input.h"

CSceneWidget::CSceneWidget()
	: m_SpriteList		()
	, m_SpriteState		()
	, m_SceneName		()
	, m_RenderList		()
	, m_DeleteList		()
	, m_pLogList		( nullptr )
	, m_HitBoxColor		( Color4::Red )
	, m_IsDispHitBox	( false )
	, m_pIsCreaterLog	( nullptr )
{
}

CSceneWidget::~CSceneWidget()
{
}

//---------------------------.
// UIエディタの更新.
//---------------------------.
void CSceneWidget::UIEdtorUpdate( const bool IsDrag, const bool IsAutoSave )
{
	// ドラッグによるUIの移動処理.
	for ( auto itr = m_RenderList.rbegin(), e = m_RenderList.rend(); itr != e; ++itr ) {
		const auto& Key	= itr->first;
		const auto& No	= itr->second;

		if ( m_SpriteState[Key][No].IsLock ) continue;

		// UIを動かした場合抜ける.
		if ( m_SpriteList[Key][No]->DragUpdate( IsDrag, IsAutoSave, &m_SpriteState[Key][No] ) )
			break;;
	}
	
	// 描画順番の更新.
	RenderSort();
}

//---------------------------.
// 描画.
//---------------------------.
void CSceneWidget::Render()
{
	FastAddRender();
	for ( auto& [Key, No] : m_RenderList ) {
		// 当たり判定の描画.
		if ( !m_SpriteState[Key][No].IsLock && m_IsDispHitBox )
			m_SpriteList[Key][No]->HitBoxRender( m_HitBoxColor, &m_SpriteState[Key][No] );

		// UIの描画.
		m_SpriteList[Key][No]->RenderUI( &m_SpriteState[Key][No] );
	}
	LateAddRender();
}

//---------------------------.
// UIエディタのImguiの描画.
//---------------------------.
void CSceneWidget::UIEdtorImGuiRender( const bool& IsAutoSave )
{
	// 表示しているUIを表示していく.
	for ( auto itr = m_RenderList.rbegin(), e = m_RenderList.rend(); itr != e; ++itr ) {
		const auto& Key = itr->first;
		const auto& No	= itr->second;

		// 削除ボタン.
		if ( ImGuiManager::Button( std::string( "x##Del_" + Key + std::to_string( No ) ).c_str(), Color4::Red ) ) {
			// シーン情報を保存する.
			if ( IsAutoSave ) {
				if ( FAILED( AllSpriteStateDataSave() ) ) return;
				Log::PushLog( "自動で" + m_SceneName + "シーンのスプライト情報を全て保存しました。" );
			}
			// 指定したスプライトの削除.
			m_SpriteList[Key][No]->DispSpriteDeleteNum( IsAutoSave, m_SceneName, No, &m_SpriteState[Key][No] );
			m_SpriteList[Key].erase( m_SpriteList[Key].begin() + No );
			m_SpriteState[Key].erase( m_SpriteState[Key].begin() + No );

			// 全て削除した.
			if ( m_SpriteList[Key].size() == 0 ) {
				m_DeleteList.emplace_back( Key );
			}

			// 描画順番を調べなおす.
			RenderSort();
			return;
		}

		// 上に移動ボタン.
		ImGui::SameLine();
		if ( ImGuiManager::Button( std::string( "^##Up_" + Key + std::to_string( No ) ).c_str() ) ) {
			m_SpriteState[Key][No].UIAnimState.StartTransform.Position.z	+= 0.001f;
			RenderSort();
		}
		
		// 下に移動ボタン.
		ImGui::SameLine();
		if ( ImGuiManager::Button( std::string( "v##Down_" + Key + std::to_string( No ) ).c_str() ) ) {
			m_SpriteState[Key][No].UIAnimState.StartTransform.Position.z	-= 0.001f;
			RenderSort();
		}

		// 鍵チェックボックス.
		ImGui::SameLine();
		bool Flag = m_SpriteState[Key][No].IsLock;
		ImGuiManager::CheckBox( std::string( "##LockCheckbox_" + Key + std::to_string( No ) ).c_str(), &Flag, Color4::Gray );
		m_SpriteState[Key][No].IsLock = Flag;

		// 表示しているUIの名前のツリーを作成.
		ImGui::SameLine();
		if ( ImGui::TreeNode( std::string( Key + "_" + std::to_string( No ) ).c_str() ) ) {
			ImGui::Separator();
			ImGuiManager::CheckBox( std::string( u8"表示するか##Checkbox_" + Key + std::to_string( No ) ).c_str(), &m_SpriteState[Key][No].IsDisp );

			// UIアニメーションの設定の表示.
			m_SpriteList[Key][No]->UIEdtorImGuiRender( IsAutoSave, &m_SpriteState[Key][No] );
			ImGui::TreePop();
		}
		ImGui::Separator();
	}
}

//--------------------------.
// UIスプライトの追加.
//--------------------------.
void CSceneWidget::AddUISpreite( const std::string& Name, CSprite* pSprite )
{
	// スプライトの追加.
	m_SpriteList[Name].emplace_back( pSprite );
	m_SpriteState[Name].emplace_back( pSprite->GetRenderState() );
	m_SpriteList[Name].back()->SetIsCreaterLog( m_pIsCreaterLog );
	m_SpriteList[Name].back()->SetLogList( m_pLogList );

	// 削除リストに追加するスプライトがあるか.
	auto itr = std::find( m_DeleteList.begin(), m_DeleteList.end(), Name );
	if ( itr != m_DeleteList.end() ) {
		// 削除リストから削除する.
		const __int64 No = std::distance( m_DeleteList.begin(), itr );
		m_DeleteList.erase( m_DeleteList.begin() + No );
	}

	// 描画順番を更新する.
	RenderSort();
}

//---------------------------.
// 表示しているスプライト全てのスプライト情報を保存する.
//---------------------------.
HRESULT CSceneWidget::AllSpriteStateDataSave()
{
	// 情報を全て保存する.
	for ( auto& [Key, No] : m_RenderList ) {
		if ( FAILED( m_SpriteList[Key][No]->SpriteStateDataSave( &m_SpriteState[Key][No] ) ) ) return E_FAIL;
	}

	// 削除したスプライトの保存.
	for ( auto& l : m_DeleteList ) {
		SpriteResource::GetSprite( l )->SpriteStateSaveReload();
	}
	m_DeleteList.clear();

	return S_OK;
}

//--------------------------.
// 表示しているスプライト全てのスプライト情報を読み込む.
//--------------------------.
HRESULT CSceneWidget::AllSpriteStateDataLoad()
{
	for ( auto& [Key, List] : m_SpriteList ) {
		if ( List.empty()								) continue;
		if ( FAILED( List[0]->SpriteStateDataLoad() )	) return E_FAIL;
	}
	return S_OK;
}

//---------------------------.
// シーン名の設定.
//---------------------------.
void CSceneWidget::SetSceneName( const ESceneList& NowScene )
{
	m_SceneName = StringConversion::Enum( NowScene );
}

//---------------------------.
// スプライトリストを取得する.
//---------------------------.
void CSceneWidget::GetSpriteList()
{
	const std::vector<std::string>& Resource = SpriteResource::GetSpriteNames();

	// このシーンで読み込む画像を取得する.
	for ( auto& r : Resource ) {
		CSprite*				 pSprite	= SpriteResource::GetSprite( r );
		pSprite->SpriteStateDataLoad();
		std::vector<std::string> SceneList	= pSprite->GetDispSceneList();
		bool					 IsDisp		= false;
		for ( auto& s : SceneList ) {
			if ( s == m_SceneName ) {
				IsDisp = true;

				// 表示する個数を取得する.
				const int& Num = pSprite->GetSceneDispNum( s );
				m_SpriteList[r].clear();
				m_SpriteState[r].clear();
				m_SpriteList[r].resize( Num );
				m_SpriteState[r].resize( Num );
				for ( int i = 0; i < Num; ++i ) {
					m_SpriteList[r][i]	= pSprite;
					m_SpriteList[r][i]->InitSceneSpriteState( s, i );
					m_SpriteState[r][i]	= m_SpriteList[r][i]->GetRenderState();
				}
				break;
			}
		}
		// 表示していない場合削除する.
		if ( IsDisp == false ) {
			m_SpriteList.erase( r );
			m_SpriteState.erase( r );
		}
	}
	// 描画順番のリストを作成.
	RenderSort();
}

//---------------------------.
// 描画順番のソート.
//---------------------------.
void CSceneWidget::RenderSort()
{
	// Z座標のリストの作成.
	std::vector<std::pair<float, std::pair<std::string, int>>> ZList;
	for ( auto& [Key, State] : m_SpriteState ) {
		const int& Num = static_cast<int>( State.size() );
		for ( int i = 0; i < Num; ++i )
			ZList.emplace_back( std::make_pair( State[i].UIAnimState.StartTransform.Position.z, std::make_pair( Key, i ) ) );
	}
	// Z座標が小さい順に並べ替える.
	std::sort( ZList.begin(), ZList.end() );

	// 描画順番のリストを作成.
	const int& Size = static_cast<int>( ZList.size() );
	m_RenderList.resize( Size );
	for ( int i = 0; i < Size; ++i ) {
		m_RenderList[i] = ZList[i].second;
	}
}
