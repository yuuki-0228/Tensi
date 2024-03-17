#pragma once
#include "..\GameObject.h"
#include "..\..\..\Common\Sprite\Sprite.h"
#include "..\..\..\Common\Font\Font.h"
#include "..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\Resource\FontResource\FontResource.h"

namespace {
	// �I�����񋓑�.
	enum class enChoice : unsigned char {
		StartChoice,	// �ŏ��̑I����.
		EndChoice,		// �Ō�̑I����.

		Max,			// �ő�v�f��.
	} typedef EChoice;

	// �^�C�g���̑I�����񋓑�.
	enum class enTitleChoice : unsigned char
	{
		GameStart,			// �Q�[���J�n.
		MiniGame,			// �~�j�Q�[��.
		PizzaEdit,			// �s�U�G�f�B�^.
		RankingDelete,		// �����L���O�̍폜.
		GameEnd,			// �Q�[���I��.

		Max,				// �ő�v�f��.
		Start	= 0,		// ��ԍŏ��̑I����.
		End		= Max - 1,	// ��ԍŌ�̑I����.
	} typedef ETitleChoice;

	// �g�k�Ǘ��񋓑�.
	enum class enScaling : unsigned char
	{
		None,			// �ҋ@��.
		Expand,			// �g�咆.
		Shrink,			// �k����.
	} typedef EScaling;
}

/************************************************
*	�Q�[������UI�x�[�X�N���X.
*		���c�F�P.
**/
class CWidget
	: public CGameObject
{
public:
	using Sprite_map	= std::unordered_map<std::string, std::vector<CSprite*>>;
	using State_map		= std::unordered_map<std::string, SSpriteRenderStateList>;

public:
	CWidget();
	virtual ~CWidget();

	// �`��.
	virtual void Sprite3DRender_A() override final {};
	virtual void Sprite3DRender_B() override final {};

protected:
	const float m_Wnd_W;		// �E�B���h�E�̕�.
	const float m_Wnd_H;		// �E�B���h�E�̍���.
};
