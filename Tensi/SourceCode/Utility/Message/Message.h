#pragma once
#include "..\..\Global.h"

namespace Msg{
	// ���b�Z�[�W�^�C�v�񋓑�
	enum class enMessageType {
		// �����t���[�����b�Z�[�W.
		//	���̃t���[���Ŏ����I�ɍ폜����郁�b�Z�[�W.
		OneFrameMsg,
		// �`�F�b�N�t���[�����b�Z�[�W.
		//	�m�F�������̃t���[���Ŏ����I�ɍ폜����郁�b�Z�[�W.
		//	�m�F��A�����t���[�����b�Z�[�W�Ɏ����I�ɐ؂�ւ�邽��.
		//	�I�[�g���b�Z�[�W�Ŋm�F���Ă�������.
		CheckFrameMsg,
		// �`�F�b�N���b�Z�[�W.
		//	�m�F����܂Ŏc�葱���郁�b�Z�[�W.
		CheckMsg,
		// �j���[���b�Z�[�W.
		//	�f���[�g���b�Z�[�W���͂��܂Ŏc�葱���郁�b�Z�[�W.
		NewMsg,
		// �f���[�g���b�Z�[�W.
		//	�j���[���b�Z�[�W���폜���郁�b�Z�[�W.
		DeleteMsg,
		// �I�[�g���b�Z�[�W.
		//	�m�F�����b�Z�[�W�^�C�v�������Ŕ��f���郁�b�Z�[�W.
		AutoMsg,

		None
	} typedef EMsgType;

	constexpr char ONE_FRAME_MSG[]	 = "FM_";	// �����t���[�����b�Z�[�W���ʕ���.
	constexpr char CHECK_FRAME_MSG[] = "LM_";	// �`�F�b�N�t���[�����b�Z�[�W���ʕ���.
	constexpr char CHECK_MSG[]		 = "CM_";	// �`�F�b�N���b�Z�[�W���ʕ���.
	constexpr char NEW_MSG[]		 = "NM_";	// �j���[���b�Z�[�W���ʕ���.
	constexpr char DELETE_MSG[]		 = "DM_";	// �f���[�g���b�Z�[�W���ʕ���.
	constexpr char AUTO_MSG[]		 = "AM_";	// �I�[�g���b�Z�[�W���ʕ���.
}

/************************************************
*	���b�Z�[�W�N���X.
**/
class Message final
{
public:
	Message();
	~Message();

	// �X�V.
	static void Update( const float& DeltaTime );

	// ���b�Z�[�W�𑗂�.
	static void Send( const std::string& Message, const Msg::EMsgType Type = Msg::EMsgType::CheckFrameMsg );

	// ���b�Z�[�W�̊m�F.
	static bool Check( const std::string& Message, const Msg::EMsgType Type = Msg::EMsgType::AutoMsg );

private:
	// �C���X�^���X�̎擾.
	static Message* GetInstance();

	// �j���[���b�Z�[�W���폜.
	static bool NewMsgDelete( const std::string& CheckMsg );

	// �`�F�b�N�t���[�����b�Z�[�W�̕ύX.
	static bool CheckFrameMsgChange( const std::string& CheckMsg );

	// �`�F�b�N���b�Z�[�W���폜.
	static bool CheckMsgDelete( const std::string& CheckMsg );

	// �I�[�g���b�Z�[�W�̊m�F.
	static bool AutoMsgCheck( const std::string& CheckMsg );

private:
	std::unordered_map<std::string, bool> m_MsgList;
};