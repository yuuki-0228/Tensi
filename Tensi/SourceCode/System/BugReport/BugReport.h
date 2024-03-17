#pragma once
#include "..\SystemBase.h"

/************************************************
*	�o�O�񍐃N���X.
*		���c�F�P.
**/
class CBugReport final
	: public CSystemBase
{
public:
	using BugList_map = std::unordered_map<std::string, std::vector<std::string>>;

private:
	// �o�O���x���񋓑�.
	enum class enBugLv : unsigned char {
		S,	// �Q�[���i�s�ɖ�肪�o��( ���ʂɃQ�[�����Ă��đ������� ).
		A,	// �v���C���[�ɒ������s���v�������炷( ����̎菇�Ŕ������� ).
		B,	// �v���C���[�ɂ�����x�s���v�������炷( �Q�[���i�s�ɖ��͂Ȃ� ).
		C,	// ���炩�ɕs������v���I�ł͂Ȃ�.
		D	// �s��Ƃ܂ł͂����Ȃ����A�C�ɂȂ�.
	} typedef EBugLv;

public:
	CBugReport();
	~CBugReport();

	// �o�O���������擾.
	bool GetBugListEmpty() { return m_BugList.empty(); }

protected:
	// �������֐�.
	virtual void Init() override;

private:
	// �o�O���X�g��ǂݍ���.
	HRESULT BugListLoad();

private:
	std::string	m_BugTitle;		// �o�O�̃^�C�g��.
	std::string	m_BugContents;	// �o�O�̐���.
	std::string	m_Discoverer;	// ������.
	EBugLv		m_BugLv;		// �o�O�̃��x��.
	BugList_map	m_BugList;		// �o�O���X�g.
};
