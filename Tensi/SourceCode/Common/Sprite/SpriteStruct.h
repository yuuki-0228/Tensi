#pragma once
#include "..\..\Global.h"

// ���[�J�����W�̔ԍ�.
enum class enLocalPosition : unsigned char
{
	LeftUp,		// 0:����.
	Left,		// 1:��.
	LeftDown,	// 2:����.
	Down,		// 3:��.
	RightDown,	// 4:�E��.
	Right,		// 5:�E.
	RightUp,	// 6:�E��.
	Up,			// 7:��.
	Center,		// 8:����.

	Max,
	None = 0
} typedef ELocalPosition;

// �T���v���X�e�[�g.
enum class enSamplerState : unsigned char
{
	Wrap,		// �^�C����ɌJ��Ԃ����.
	Mirror,		// ���]���Ȃ���J��Ԃ����.
	Clamp,		// 0~1�ɐݒ肳���.
	Border,		// 0~1�͈̔͊O��hlsl�Ŏw�肵���F�ɂȂ�.
	MirrorOne,	// 0�𒆐S�ɔ��]�����.

	Max,
	None = 0
} typedef ESamplerState;

// �R���X�^���g�o�b�t�@�̃A�v�����̒�`.
//	���V�F�[�_���̃R���X�^���g�o�b�t�@�ƈ�v���Ă���K�v����.
struct stSpriteShaderConstantBuffer
{
	ALIGN16 D3DXMATRIX	mWVP;				// ���[���h,�r���[,�v���W�F�N�V�����̍����ϊ��s��.	
	ALIGN16 D3DXCOLOR4	vColor;				// �J���[(RGBA�̌^�ɍ��킹��).
	ALIGN16 D3DXVECTOR4	vUV;				// UV���W(x,y�̂ݎg�p).
	ALIGN16 D3DXVECTOR4	vRenderArea;		// �`�悷��G���A.
	ALIGN16 float		fViewPortWidth;		// �r���[�|�[�g��.
	ALIGN16 float		fViewPortHeight;	// �r���[�|�[�g����.
	ALIGN16 D3DXVECTOR4 vDitherFlag;		// �f�B�U�������g�p���邩(x��1�ȏ�Ȃ�g�p����).
	ALIGN16 D3DXVECTOR4 vAlphaBlockFlag;	// �A���t�@�u���b�N���g�p���邩(x��1�ȏ�Ȃ�g�p����).
} typedef SSpriteShaderConstantBuffer;

// �������\����.
struct stSize
{
	float w;
	float h;

	stSize()
		: stSize ( 0.0f, 0.0f )
	{
	}
	stSize( const float size[2] )
		: w	( size[0] )
		, h	( size[1] )
	{
	}
	stSize( const float w, const float h )
		: w	( w )
		, h	( h )
	{
	}
} typedef SSize;

// ���_�̍\����.
struct stVertex
{
	D3DXPOSITION3	Pos;	// ���_���W.
	D3DXVECTOR2		Tex;	// �e�N�X�`�����W.
} typedef SVertex;

//�X�v���C�g�\����.
struct stSpriteState
{
	SSize									Disp;				// �\��������.
	SSize									Base;				// ���摜������.
	SSize									Stride;				// 1�R�}������̕�����.
	D3DXVECTOR2								OffSet;				// �␳�l.
	D3DXPOSITION3							AddCenterPos;		// ���S���W�ɕϊ��p�̒ǉ����W.
	ELocalPosition							LocalPosNo;			// ���[�J�����W�̔ԍ�.
	std::vector<std::string>				DispSceneList;		// �\������V�[����.
	std::unordered_map<std::string, int>	SceneDispNum;		// �V�[���ɕ\�������.
	std::string								FilePath;			// �t�@�C���p�X.
} typedef SSpriteState;

// UI�A�j���[�V�������\����.
struct stUIAnimState
{
	std::string		UIAnimWhen;			// ���s����.
	std::string		UIAnimWhat;			// �����s����.
	std::string		EndMessage;			// �Đ��I�����̃��b�Z�[�W��.
	STransform		StartTransform;		// �����̃g�����X�t�H�[��.
	D3DXCOLOR4		StartColor;			// �����̐F.
	bool			Flag_x;				// x�����̃t���O.
	bool			Flag_y;				// y�����̃t���O.
	bool			IsLoop;				// ���[�v�����邩.
	bool			IsAnimPlay;			// �A�j���[�V���������邩.
	float			AnimCnt;			// �A�j���[�V�����p�̃J�E���g.
	float			Cnt_x;				// x�����̃J�E���g.
	float			Cnt_y;				// y�����̃J�E���g.
	float			Speed_x;			// x�����̑��x.
	float			Speed_y;			// y�����̑��x.
	int				PlayCnt;			// �Đ��J�E���g.
	int				PlayMax;			// �Đ����.

	stUIAnimState()
		: UIAnimWhen		( "None" )
		, UIAnimWhat		( "None" )
		, EndMessage		( "" )
		, StartTransform	()
		, StartColor		( Color4::White )
		, Flag_x			( false )
		, Flag_y			( false )
		, IsLoop			( true )
		, IsAnimPlay		( false )
		, AnimCnt			( 0.0f )
		, Cnt_x				( 0.0f )
		, Cnt_y				( 0.0f )
		, Speed_x			( 0.0f )
		, Speed_y			( 0.0f )
		, PlayCnt			( 0 )
		, PlayMax			( 0 )
	{}
} typedef SUIAnimState;

// �X�v���C�g�A�j���[�V�������\����.
struct stSpriteAnimState
{
	POINT			PatternNo;				// �p�^�[���ԍ�.
	POINT			PatternMax;				// �p�^�[���ő吔.
	D3DXVECTOR2		UV;						// �e�N�X�`��UV���W.
	D3DXVECTOR2		Scroll;					// UV�X�N���[��.
	D3DXVECTOR2		ScrollSpeed;			// �e�N�X�`��UV���W.
	int				AnimNumber;				// �A�j���[�V�����ԍ�.
	int				AnimNumberMax;			// �A�j���[�V�����ԍ��̍ő吔.
	int				FrameCount;				// �t���[���J�E���g.
	int				FrameCountSpeed;		// �t���[���J�E���g���x.
	int				FrameTime;				// �t���[���^�C��.
	int				AnimPlayMax;			// �A�j���[�V�����̍s���ő吔(0:������).
	int				AnimPlayCnt;			// �A�j���[�V�����̍s������.
	bool			IsSetAnimNumber;		// �A�j���[�V�����ԍ���ݒ肷�邩.
	bool			IsSetPatternNo;			// �p�^�[���ԍ���ݒ肷�邩.
	bool			IsAnimation;			// �A�j���[�V�������邩.
	bool			IsAnimationX;			// X�����ɃA�j���[�V�������邩.
	bool			IsAnimationY;			// Y�����ɃA�j���[�V�������邩.
	bool			IsUVScrollX;			// X������UV�X�N���[�������邩.
	bool			IsUVScrollY;			// Y������UV�X�N���[�������邩.

	stSpriteAnimState()
		: PatternNo			( { 1, 1 } )
		, PatternMax		( { 1, 1 } )
		, UV				( 0.0f, 0.0f )
		, ScrollSpeed		( 0.0f, 0.0f )
		, AnimNumber		( 0 )
		, AnimNumberMax		( 1 )
		, FrameCount		( 1 )
		, FrameCountSpeed	( 1 )
		, FrameTime			( 5 )
		, AnimPlayMax		( 0 )
		, AnimPlayCnt		( 0 )
		, IsSetAnimNumber	( false )
		, IsSetPatternNo	( false )
		, IsAnimation		( false )
		, IsAnimationX		( false )
		, IsAnimationY		( false )
		, IsUVScrollX		( false )
		, IsUVScrollY		( false )
	{}
	
	//--------------------------.
	// UV���W�ɕϊ�.
	//--------------------------.
	D3DXVECTOR2 ConvertUV() {
		return {
			// x���W.
			static_cast<float>( PatternNo.x ) / static_cast<float>( PatternMax.x ),
			// y���W.
			static_cast<float>( PatternNo.y ) / static_cast<float>( PatternMax.y )
		};
	}

	//--------------------------.
	// �A�j���[�V�����ԍ��̐ݒ�.
	//	�A�j���[�V�����ԍ���ݒ�����Ȃ��ꍇ�͏����͍s��Ȃ�.
	//--------------------------.
	void SetAnimNumber() {
		if ( IsSetAnimNumber == false ) return;

		// �A�j���[�V���������Ȃ�.
		IsAnimation = false;

		// �A�j���[�V�����ԍ����Z�o.
		PatternNo.x = AnimNumber % PatternMax.x;
		PatternNo.y = AnimNumber / PatternMax.x;

		// UV���W�ɕϊ�.
		UV = ConvertUV();
	}

	//--------------------------.
	// �A�j���[�V�����ԍ��̐ݒ�.
	//	�A�j���[�V�����ԍ���ݒ�����Ȃ��ꍇ�͏����͍s��Ȃ�.
	//--------------------------.
	void SetPatternNo() {
		if ( IsSetPatternNo == false ) return;

		// �A�j���[�V���������Ȃ�.
		IsAnimation = false;

		// UV���W�ɕϊ�.
		UV = ConvertUV();
	}

	//--------------------------.
	// �A�j���[�V�����̍X�V.
	//	�A�j���[�V�������Ȃ��ꍇ�͏����͍s��Ȃ�.
	//--------------------------.
	void AnimUpdate() {
		if( IsAnimation == false && IsAnimationX == false && IsAnimationY == false ) return;

		if( FrameCount % FrameTime == 0 ){
			if ( IsAnimation ) {
				// �A�j���[�V�����ԍ����Z�o.
				PatternNo.x = AnimNumber % PatternMax.x;
				PatternNo.y = AnimNumber / PatternMax.x;
				AnimNumber++;	// �A�j���[�V�����ԍ������Z.
				if ( AnimNumber >= AnimNumberMax ) {
					// �A�j���[�V�����ԍ����ő�A�j���[�V��������葽�����.
					// ����������.
					FrameCount = 0;
					AnimNumber = 0;

					// �A�j���[�V�����̎g�p�񐔐���
					if ( AnimPlayMax > 0 ) {
						AnimPlayCnt++;
						if ( AnimPlayCnt >= AnimPlayMax ) {
							AnimPlayCnt = 0;
							IsAnimation = false;
						}
					}
				}
			}
			else if ( IsAnimationX ) {
				// �A�j���[�V�����ԍ����Z�o.
				PatternNo.x++;
				if ( PatternNo.x >= PatternMax.x ) {
					// �A�j���[�V�����ԍ����ő�A�j���[�V��������葽�����.
					// ����������.
					PatternNo.x = 0;
					FrameCount  = 0;
					AnimNumber  = 0;

					// �A�j���[�V�����̎g�p�񐔐���
					if ( AnimPlayMax > 0 ) {
						AnimPlayCnt++;
						if ( AnimPlayCnt >= AnimPlayMax ) {
							AnimPlayCnt  = 0;
							IsAnimationX = false;
						}
					}
				}
			}
			else if ( IsAnimationY ) {
				// �A�j���[�V�����ԍ����Z�o.
				PatternNo.y++;
				if ( PatternNo.y >= PatternMax.y ) {
					// �A�j���[�V�����ԍ����ő�A�j���[�V��������葽�����.
					// ����������.
					PatternNo.y = 0;
					FrameCount  = 0;
					AnimNumber  = 0;

					// �A�j���[�V�����̎g�p�񐔐���
					if ( AnimPlayMax > 0 ) {
						AnimPlayCnt++;
						if ( AnimPlayCnt >= AnimPlayMax ) {
							AnimPlayCnt  = 0;
							IsAnimationY = false;
						}
					}
				}
			}
		}
		FrameCount += FrameCountSpeed;
		// UV���W�ɕϊ�.
		UV = ConvertUV();
	}

	//--------------------------.
	// UV�X�N���[���̍X�V.
	//--------------------------.
	void UVScrollUpdate()
	{
		if ( IsUVScrollX ) {
			UV.x += ScrollSpeed.x;
			if ( UV.x > 1.0f ) UV.x -= 1.0f;
			if ( UV.x < 0.0f ) UV.x += 1.0f;
		}

		if ( IsUVScrollY ) {
			UV.y += ScrollSpeed.y;
			if ( UV.y > 1.0f ) UV.y -= 1.0f;
			if ( UV.y < 0.0f ) UV.y += 1.0f;
		}
	}

} typedef SSpriteAnimState;

// �`��p�摜���\����.
struct stSpriteRenderState
{
	SSpriteAnimState	AnimState;			// �A�j���[�V�������.
	SUIAnimState		UIAnimState;		// UI�A�j���[�V�������.
	STransform			Transform;			// �g�����X�t�H�[��.
	ESamplerState		SmaplerNo;			// �T���v���ԍ�.
	D3DXCOLOR4			Color;				// �F.
	D3DXVECTOR4			RenderArea;			// �`�悷��G���A( ����x, ����y, ��, ���� ).
	std::string			SceneName;			// �ǂ̃V�[����.
	std::string			No;					// �ǂ̐ݒ�̉摜��.
	bool				IsDisp;				// �\�����邩.
	bool				IsGrab;				// ���܂�Ă��邩.
	bool				IsLock;				// ���߂Ȃ����邩.

	stSpriteRenderState()
		: AnimState		()
		, UIAnimState	()
		, Transform		()
		, SmaplerNo		( ESamplerState::Wrap )
		, Color			( Color4::White )
		, RenderArea	( 0.0f, 0.0f, -1.0f, -1.0f )
		, SceneName		( "" )
		, No			( "" )
		, IsDisp		( true )
		, IsGrab		( false )
		, IsLock		( false )
	{}

} typedef SSpriteRenderState;
using SSpriteRenderStateList = std::vector<SSpriteRenderState>;