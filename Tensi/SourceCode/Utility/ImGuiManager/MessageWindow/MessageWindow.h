#pragma once
#include "..\..\..\Global.h"
#include "..\ImGuiManager.h"

/************************************************
*	���b�Z�[�W�E�B���h�E�N���X.
*		���c�F�P.
**/
class MessageWindow final
{
public:
	MessageWindow();
	~MessageWindow();

	// �`��.
	static void Render();

	// ���b�Z�[�W�E�B���h�E�ɒǉ�.
	static void PushMessage( const std::string& Message, const D3DXCOLOR3& Color );
	static void PushMessage( const std::string& Message, const D3DXCOLOR4& Color = Color4::White );
	
private:
	// �C���X�^���X�̎擾.
	static MessageWindow* GetInstance();

	static void MessageFind();		// ���b�Z�[�W��������.
	static void ColorBlock();		// �J���[�u���b�N����.
	void SetColorTable();			// �J���[�e�[�u���̐ݒ�.

private:
	std::vector<std::function<void()>>	m_MessageList;		// ���b�Z�[�W�E�B���h�E�p.
	std::vector<bool>					m_ColorBlockFlags;	// �e�J���[���u���b�N���Ȃ����̃t���O.
	std::vector<D3DXCOLOR4>				m_ColorTable;		// �F�̃e�[�u��.
	std::string							m_FindString;		// ���b�Z�[�W�E�B���h�E�̌������镶��.
	bool								m_IsFindTime;		// ���b�Z�[�W�E�B���h�E�̌����Ɏ��Ԃ��܂܂��邩.
	bool								m_IsNewMsg;			// ���b�Z�[�W�ǉ����ꂽ��.
	int									m_ColorResetCnt;	// �F�����ɖ߂��J�E���g.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	MessageWindow( const MessageWindow & )				= delete;
	MessageWindow& operator = ( const MessageWindow & )	= delete;
	MessageWindow( MessageWindow && )						= delete;
	MessageWindow& operator = ( MessageWindow && )		= delete;
};
