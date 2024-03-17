#pragma once
#include "..\..\Global.h"
#include "..\Sprite\SpriteStruct.h"

// �R���X�^���g�o�b�t�@�̃A�v�����̒�`.
//	���V�F�[�_���̃R���X�^���g�o�b�t�@�ƈ�v���Ă���K�v����.
struct stFontShaderConstantBuffer
{
	ALIGN16 D3DXMATRIX	mWVP;				// ���[���h,�r���[,�v���W�F�N�V�����̍����ϊ��s��.	
	ALIGN16 D3DXCOLOR4	vColor;				// �J���[(RGBA�̌^�ɍ��킹��).
	ALIGN16 D3DXVECTOR4	vUV;				// UV���W(x,y�̂ݎg�p).
	ALIGN16 D3DXVECTOR4	vRenderArea;		// �`�悷��G���A.
	ALIGN16 float		fViewPortWidth;		// �r���[�|�[�g��.
	ALIGN16 float		fViewPortHeight;	// �r���[�|�[�g����.
	ALIGN16 D3DXVECTOR4 vDitherFlag;		// �f�B�U�������g�p���邩(x��1�ȏ�Ȃ�g�p����).
	ALIGN16 D3DXVECTOR4 vAlphaBlockFlag;	// �A���t�@�u���b�N���g�p���邩(x��1�ȏ�Ȃ�g�p����).
} typedef SFontShaderConstantBuffer;

//�X�v���C�g�\����.
struct stFontState
{
	SSize			Disp;				// �\��������.
	ELocalPosition	LocalPosNo;			// ���[�J�����W�̔ԍ�.
} typedef SFontState;

// �e�L�X�g�̕\���`��.
enum class enTextAlign : unsigned char
{
	Left,		// ������.
	Center,		// ��������.
	Right		// �E����.
} typedef ETextAlign;

// �`��p�摜���\����.
struct stFontRenderState
{
	STransform			Transform;			// �g�����X�t�H�[��.
	std::string			Text;				// �`�悷��e�L�X�g.
	ESamplerState		SmaplerNo;			// �T���v���ԍ�.
	ETextAlign			TextAlign;			// �e�L�X�g�̕\���`��.
	D3DXCOLOR4			Color;				// �F.
	D3DXCOLOR4			OutLineColor;		// �A�E�g���C���̐F.
	D3DXVECTOR4			RenderArea;			// �`�悷��G���A( ����x, ����y, ��, ���� ).
	SSize				TextInterval;		// �����ʂ��̊Ԋu.
	float				OutLineSize;		// �A�E�g���C���̑���.
	bool				IsDisp;				// �\�����邩.
	bool				IsOutLine;			// �A�E�g���C�����g�p���邩.
	bool				IsBold;				// �������ɂ��邩.

	stFontRenderState()
		: Transform		()
		, Text			( "" )
		, SmaplerNo		( ESamplerState::Wrap )
		, TextAlign		( ETextAlign::Left )
		, Color			( Color4::White )
		, OutLineColor	( Color4::Black )
		, RenderArea	( 0, 0, WND_W, WND_H )
		, TextInterval	( 0.0f, 0.0f )
		, OutLineSize	( 4.0 )
		, IsDisp		( true )
		, IsOutLine		( false )
		, IsBold		( false )
	{}

} typedef SFontRenderState;
using SFontRenderStateList = std::vector<SFontRenderState>;