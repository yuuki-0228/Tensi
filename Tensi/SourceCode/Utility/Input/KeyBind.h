#pragma once
#include <Xinput.h>

// �L�[�o�C���h�̐ݒ�Ŏg�p.
namespace{
	constexpr WORD XINPUT_LSTICK_U = 0x0050;	// �� �X�e�B�b�N�� ���������
	constexpr WORD XINPUT_LSTICK_D = 0x0051;	// �� �X�e�B�b�N�� ����������.
	constexpr WORD XINPUT_LSTICK_L = 0x0052;	// �� �X�e�B�b�N�� ����������.
	constexpr WORD XINPUT_LSTICK_R = 0x0053;	// �� �X�e�B�b�N�� �E��������.
	constexpr WORD XINPUT_RSTICK_U = 0x0054;	// �E �X�e�B�b�N�� ���������.
	constexpr WORD XINPUT_RSTICK_D = 0x0055;	// �E �X�e�B�b�N�� ����������.
	constexpr WORD XINPUT_RSTICK_L = 0x0056;	// �E �X�e�B�b�N�� ����������.
	constexpr WORD XINPUT_RSTICK_R = 0x0057;	// �E �X�e�B�b�N�� �E��������.
	constexpr WORD XINPUT_LTRIGGER = 0x0058;	// �� �g���K�[��	  ����.
	constexpr WORD XINPUT_RTRIGGER = 0x0059;	// �E �g���K�[��	  ����.
}

// �L�[�o�C���h�̃y�A�\����.
struct stKeyBindPair {
	std::vector<int>	Key;	// �L�[.
	std::vector<WORD>	But;	// Xinput�̃{�^��.

} typedef SKeyBindPair;

// �L�[�o�C���h�̃}�b�v.
using KeyBind_map = std::unordered_map<std::string, SKeyBindPair>;
