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
// UI�G�f�B�^�̍X�V.
//---------------------------.
void CSceneWidget::UIEdtorUpdate( const bool IsDrag, const bool IsAutoSave )
{
	// �h���b�O�ɂ��UI�̈ړ�����.
	for ( auto itr = m_RenderList.rbegin(), e = m_RenderList.rend(); itr != e; ++itr ) {
		const auto& Key	= itr->first;
		const auto& No	= itr->second;

		if ( m_SpriteState[Key][No].IsLock ) continue;

		// UI�𓮂������ꍇ������.
		if ( m_SpriteList[Key][No]->DragUpdate( IsDrag, IsAutoSave, &m_SpriteState[Key][No] ) )
			break;;
	}
	
	// �`�揇�Ԃ̍X�V.
	RenderSort();
}

//---------------------------.
// �`��.
//---------------------------.
void CSceneWidget::Render()
{
	FastAddRender();
	for ( auto& [Key, No] : m_RenderList ) {
		// �����蔻��̕`��.
		if ( !m_SpriteState[Key][No].IsLock && m_IsDispHitBox )
			m_SpriteList[Key][No]->HitBoxRender( m_HitBoxColor, &m_SpriteState[Key][No] );

		// UI�̕`��.
		m_SpriteList[Key][No]->RenderUI( &m_SpriteState[Key][No] );
	}
	LateAddRender();
}

//---------------------------.
// UI�G�f�B�^��Imgui�̕`��.
//---------------------------.
void CSceneWidget::UIEdtorImGuiRender( const bool& IsAutoSave )
{
	// �\�����Ă���UI��\�����Ă���.
	for ( auto itr = m_RenderList.rbegin(), e = m_RenderList.rend(); itr != e; ++itr ) {
		const auto& Key = itr->first;
		const auto& No	= itr->second;

		// �폜�{�^��.
		if ( ImGuiManager::Button( std::string( "x##Del_" + Key + std::to_string( No ) ).c_str(), Color4::Red ) ) {
			// �V�[������ۑ�����.
			if ( IsAutoSave ) {
				if ( FAILED( AllSpriteStateDataSave() ) ) return;
				Log::PushLog( "������" + m_SceneName + "�V�[���̃X�v���C�g����S�ĕۑ����܂����B" );
			}
			// �w�肵���X�v���C�g�̍폜.
			m_SpriteList[Key][No]->DispSpriteDeleteNum( IsAutoSave, m_SceneName, No, &m_SpriteState[Key][No] );
			m_SpriteList[Key].erase( m_SpriteList[Key].begin() + No );
			m_SpriteState[Key].erase( m_SpriteState[Key].begin() + No );

			// �S�č폜����.
			if ( m_SpriteList[Key].size() == 0 ) {
				m_DeleteList.emplace_back( Key );
			}

			// �`�揇�Ԃ𒲂ׂȂ���.
			RenderSort();
			return;
		}

		// ��Ɉړ��{�^��.
		ImGui::SameLine();
		if ( ImGuiManager::Button( std::string( "^##Up_" + Key + std::to_string( No ) ).c_str() ) ) {
			m_SpriteState[Key][No].UIAnimState.StartTransform.Position.z	+= 0.001f;
			RenderSort();
		}
		
		// ���Ɉړ��{�^��.
		ImGui::SameLine();
		if ( ImGuiManager::Button( std::string( "v##Down_" + Key + std::to_string( No ) ).c_str() ) ) {
			m_SpriteState[Key][No].UIAnimState.StartTransform.Position.z	-= 0.001f;
			RenderSort();
		}

		// ���`�F�b�N�{�b�N�X.
		ImGui::SameLine();
		bool Flag = m_SpriteState[Key][No].IsLock;
		ImGuiManager::CheckBox( std::string( "##LockCheckbox_" + Key + std::to_string( No ) ).c_str(), &Flag, Color4::Gray );
		m_SpriteState[Key][No].IsLock = Flag;

		// �\�����Ă���UI�̖��O�̃c���[���쐬.
		ImGui::SameLine();
		if ( ImGui::TreeNode( std::string( Key + "_" + std::to_string( No ) ).c_str() ) ) {
			ImGui::Separator();
			ImGuiManager::CheckBox( std::string( u8"�\�����邩##Checkbox_" + Key + std::to_string( No ) ).c_str(), &m_SpriteState[Key][No].IsDisp );

			// UI�A�j���[�V�����̐ݒ�̕\��.
			m_SpriteList[Key][No]->UIEdtorImGuiRender( IsAutoSave, &m_SpriteState[Key][No] );
			ImGui::TreePop();
		}
		ImGui::Separator();
	}
}

//--------------------------.
// UI�X�v���C�g�̒ǉ�.
//--------------------------.
void CSceneWidget::AddUISpreite( const std::string& Name, CSprite* pSprite )
{
	// �X�v���C�g�̒ǉ�.
	m_SpriteList[Name].emplace_back( pSprite );
	m_SpriteState[Name].emplace_back( pSprite->GetRenderState() );
	m_SpriteList[Name].back()->SetIsCreaterLog( m_pIsCreaterLog );
	m_SpriteList[Name].back()->SetLogList( m_pLogList );

	// �폜���X�g�ɒǉ�����X�v���C�g�����邩.
	auto itr = std::find( m_DeleteList.begin(), m_DeleteList.end(), Name );
	if ( itr != m_DeleteList.end() ) {
		// �폜���X�g����폜����.
		const __int64 No = std::distance( m_DeleteList.begin(), itr );
		m_DeleteList.erase( m_DeleteList.begin() + No );
	}

	// �`�揇�Ԃ��X�V����.
	RenderSort();
}

//---------------------------.
// �\�����Ă���X�v���C�g�S�ẴX�v���C�g����ۑ�����.
//---------------------------.
HRESULT CSceneWidget::AllSpriteStateDataSave()
{
	// ����S�ĕۑ�����.
	for ( auto& [Key, No] : m_RenderList ) {
		if ( FAILED( m_SpriteList[Key][No]->SpriteStateDataSave( &m_SpriteState[Key][No] ) ) ) return E_FAIL;
	}

	// �폜�����X�v���C�g�̕ۑ�.
	for ( auto& l : m_DeleteList ) {
		SpriteResource::GetSprite( l )->SpriteStateSaveReload();
	}
	m_DeleteList.clear();

	return S_OK;
}

//--------------------------.
// �\�����Ă���X�v���C�g�S�ẴX�v���C�g����ǂݍ���.
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
// �V�[�����̐ݒ�.
//---------------------------.
void CSceneWidget::SetSceneName( const ESceneList& NowScene )
{
	m_SceneName = StringConversion::Enum( NowScene );
}

//---------------------------.
// �X�v���C�g���X�g���擾����.
//---------------------------.
void CSceneWidget::GetSpriteList()
{
	const std::vector<std::string>& Resource = SpriteResource::GetSpriteNames();

	// ���̃V�[���œǂݍ��މ摜���擾����.
	for ( auto& r : Resource ) {
		CSprite*				 pSprite	= SpriteResource::GetSprite( r );
		pSprite->SpriteStateDataLoad();
		std::vector<std::string> SceneList	= pSprite->GetDispSceneList();
		bool					 IsDisp		= false;
		for ( auto& s : SceneList ) {
			if ( s == m_SceneName ) {
				IsDisp = true;

				// �\����������擾����.
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
		// �\�����Ă��Ȃ��ꍇ�폜����.
		if ( IsDisp == false ) {
			m_SpriteList.erase( r );
			m_SpriteState.erase( r );
		}
	}
	// �`�揇�Ԃ̃��X�g���쐬.
	RenderSort();
}

//---------------------------.
// �`�揇�Ԃ̃\�[�g.
//---------------------------.
void CSceneWidget::RenderSort()
{
	// Z���W�̃��X�g�̍쐬.
	std::vector<std::pair<float, std::pair<std::string, int>>> ZList;
	for ( auto& [Key, State] : m_SpriteState ) {
		const int& Num = static_cast<int>( State.size() );
		for ( int i = 0; i < Num; ++i )
			ZList.emplace_back( std::make_pair( State[i].UIAnimState.StartTransform.Position.z, std::make_pair( Key, i ) ) );
	}
	// Z���W�����������ɕ��בւ���.
	std::sort( ZList.begin(), ZList.end() );

	// �`�揇�Ԃ̃��X�g���쐬.
	const int& Size = static_cast<int>( ZList.size() );
	m_RenderList.resize( Size );
	for ( int i = 0; i < Size; ++i ) {
		m_RenderList[i] = ZList[i].second;
	}
}
