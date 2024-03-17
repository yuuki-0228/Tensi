#pragma once

// _stprintf�֐��Ŏg�p.
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <tchar.h>

//�x���ɂ��ẴR�[�h���͂𖳌��ɂ���.4005:�Ē�`.
#pragma warning(disable:4005)
#include <Windows.h>
#include <crtdbg.h>
#include <memory>
#include <limits>

#include <d3dx9.h>
#include <D3DX11.h>
#include <D3D11.h>
#include <D3DX10.h>
#include <D3D10.h>
#include <d3d10.h>

#include "Utility\Log\Log.h"
#include "Utility\Bool\Bool.h"
#include "Utility\Color\Color.h"
#include "Utility\Transform\Transform.h"

//���C�u�����ǂݍ���.
#pragma comment( lib, "winmm.lib"	)
#pragma comment( lib, "d3d9.lib"	)
#pragma comment( lib, "d3dx9.lib"	)
#pragma comment( lib, "d3dx11.lib"	)
#pragma comment( lib, "d3d11.lib"	)
#pragma comment( lib, "d3dx10.lib"	)

//----------------------------.
//	�萔.
//----------------------------.
// �E�B���h�E�֌W.
constexpr int		WND_W			= 1280;		// �E�B���h�E�̕�.
constexpr int		WND_H			= 720;		// �E�B���h�E�̍���.
constexpr int		FPS				= 60;		// �t���[�����[�g.
constexpr float		FWND_W			= static_cast<float>( WND_W );
constexpr float		FWND_H			= static_cast<float>( WND_H );
constexpr float		FFPS			= static_cast<float>( FPS );

// �������p.
constexpr int		INIT_INT		= 0;
constexpr int		INIT_INT2[2]	= { INIT_INT, INIT_INT };
constexpr int		INIT_INT3[3]	= { INIT_INT, INIT_INT, INIT_INT };
constexpr int		INIT_INT4[4]	= { INIT_INT, INIT_INT, INIT_INT, INIT_INT };
constexpr float		INIT_FLOAT		= 0.0f;
constexpr float		INIT_FLOAT2[2]	= { INIT_FLOAT, INIT_FLOAT };
constexpr float		INIT_FLOAT3[3]	= { INIT_FLOAT, INIT_FLOAT, INIT_FLOAT };
constexpr float		INIT_FLOAT4[4]	= { INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT };

// �G���[�p.
constexpr int		ERROR_INT		= std::numeric_limits<int>::infinity();
constexpr int		ERROR_INT2[2]	= { ERROR_INT, ERROR_INT };
constexpr int		ERROR_INT3[3]	= { ERROR_INT, ERROR_INT, ERROR_INT };
constexpr int		ERROR_INT4[4]	= { ERROR_INT, ERROR_INT, ERROR_INT, ERROR_INT };
constexpr float 	ERROR_FLOAT		= std::numeric_limits<float>::infinity();
constexpr float		ERROR_FLOAT2[2]	= { ERROR_FLOAT, ERROR_FLOAT };
constexpr float 	ERROR_FLOAT3[3]	= { ERROR_FLOAT, ERROR_FLOAT, ERROR_FLOAT };
constexpr float 	ERROR_FLOAT4[4]	= { ERROR_FLOAT, ERROR_FLOAT, ERROR_FLOAT, ERROR_FLOAT };

//----------------------------.
//	�񋓑�.
//----------------------------.
// ����.
enum class enDirection : unsigned char{
	Left,		 L	= Left,			// ��.
	Right,		 R	= Right,		// �E.
	Up,			 U	= Up,			// ��.
	Down,		 D	= Down,			// ��.

	Front,		 F	= Front,		// �O.
	Center,		 C	= Center,		// �^��.
	Back,		 B	= Back,			// ���.

	LeftUp,		 LU	= LeftUp,		// ���̏�.
	LeftDown,	 LD	= LeftDown,		// ���̉�.
	LeftFront,	 LF	= LeftFront,	// ���̑O.
	LeftCenter,	 LC	= LeftCenter,	// ���̐^��.
	LeftBack,	 LB	= LeftBack,		// ���̌��.

	RightUp,	 RU	= RightUp,		// �E�̏�.
	RightDown,	 RD	= RightDown,	// �E�̉�.
	RightFront,	 RF	= RightFront,	// �E�̑O.
	RightCenter, RC	= RightCenter,	// �E�̐^��.
	RightBack,	 RB	= RightBack,	// �E�̌��.

	UpLeft,		 UL	= UpLeft,		// ��̍�.
	UpRight,	 UR	= UpRight,		// ��̉E.
	UpFront,	 UF	= UpFront,		// ��̑O.
	UpCenter,	 UC	= UpCenter,		// ��̐^��.
	UpBack,		 UB	= UpBack,		// ��̌��.

	DownLeft,	 DL	= DownLeft,		// ���̍�.
	DownRight,	 DR	= DownRight,	// ���̉E.
	DownFront,	 DF	= DownFront,	// ���̑O.
	DownCenter,	 DC	= DownCenter,	// ���̐^��.
	DownBack,	 DB	= DownBack,		// ���̌��.

	FrontLeft,	 FL	= FrontLeft,	// �O�̍�.
	FrontRight,  FR	= FrontRight,	// �O�̉E.
	FrontUp,	 FU	= FrontUp,		// �O�̏�.
	FrontDown,	 FD	= FrontDown,	// �O�̉�.

	CenterLeft,	 CL	= CenterLeft,	// �^�񒆂̍�.
	CenterRight, CR	= CenterRight,	// �^�񒆂̉E.
	CenterUp,	 CU	= CenterUp,		// �^�񒆂̏�.
	CenterDown,	 CD	= CenterDown,	// �^�񒆂̉�.

	BackLeft,	 BL	= BackLeft,		// ���̍�.
	BackRight,	 BR	= BackRight,	// ���̉E.
	BackUp,		 BU	= BackUp,		// ���̏�.
	BackDown,	 BD	= BackDown,		// ���̉�.

	East,		 E	= East,			// ��.
	West,		 W	= West,			// ��.
	South,		 S	= South,		// ��.
	North,		 N	= North,		// �k.
	NorthEast,	 NE	= NorthEast,	// �k��.
	NorthWest,	 NW	= NorthWest,	// �k��.
	SouthEast,	 SE	= SouthEast,	// �쓌.
	SouthWest,	 SW	= SouthWest,	// �쐼.

	None,
} typedef EDirection, EDire;

// ����.
enum class enNumbers : unsigned char{
	Zero,	_0	= Zero,		// 0.
	One,	_1	= One,		// 1. 
	Two,	_2	= Two,		// 2. 
	Three,	_3	= Three,	// 3.
	Four,	_4	= Four,		// 4. 
	Five,	_5	= Five,		// 5. 
	Six,	_6	= Six,		// 6. 
	Seven,	_7	= Seven,	// 7. 
	Eight,	_8	= Eight,	// 8.
	Nine,	_9	= Nine,		// 9
	Ten,	_10	= Ten,		// 10. 

	None = 0
} typedef ENumbers, ENum;

//----------------------------.
//	�}�N��.
//----------------------------.
//���.
#define SAFE_RELEASE(p)	if(p!=nullptr){(p)->Release();(p)=nullptr;}
//�j��.
#define SAFE_DELETE(p) if(p!=nullptr){delete (p);(p)=nullptr;}
#define SAFE_DELETE_ARRAY(p)	\
{								\
	if(p!=nullptr){				\
		delete[] (p);			\
		(p) = nullptr;			\
	}							\
}

// _declspec():DLL����(�֐�,�N���X,�N���X�̃����o�֐���)�G�N�X�|�[�g����.
//	algn():(�����I��)16byte�Ŏg�p����.
#define ALIGN16 _declspec( align( 16 ) )

//----------------------------.
// �G���[���b�Z�[�W.
//----------------------------.
#include <string>
#include <codecvt>
#include <iostream>
#include <sstream>
#include "Utility\StringConversion\StringConversion.h"

template<class T = std::string>
void ErrorMessage( const std::string& ErrorMsg, const HRESULT r = S_OK )
{
	// �G���[���b�Z�[�W����̏ꍇ.
	if ( ErrorMsg.empty() ) {
		_ASSERT_EXPR( false, L"Not Error Message" );
	}
	
	// �G���[���b�Z�[�W�̕\��.
	const std::wstring wMsg = StringConversion::to_wString( ErrorMsg );
	Log::PushLog( ErrorMsg.c_str() );
	if ( FAILED( r ) ) {
		// �G���[�R�[�h�o��
		std::ostringstream ss;
		ss << "ErrorCode : 0x" << std::hex << static_cast< int >( r );
		Log::PushLog( ss.str() );
	}
	_ASSERT_EXPR( false, wMsg.c_str() );
	MessageBox( nullptr, wMsg.c_str(), _T( "Warning" ), MB_OK );

#ifndef _DEBUG
	// �t���[�Y�΍�̂��߃E�B���h�E��j������
	PostQuitMessage( 0 );
#endif
}

template<class T = std::string>
void InfoMessage( const std::string& ErrorMsg )
{
	// �G���[���b�Z�[�W����̏ꍇ.
	if ( ErrorMsg.empty() ) {
		_ASSERT_EXPR( false, L"Not Error Message" );
	}

	// �G���[���b�Z�[�W�̕\��.
	const std::wstring wMsg = StringConversion::to_wString( ErrorMsg );
	MessageBox( nullptr, wMsg.c_str(), _T( "Info" ), MB_OK );
}