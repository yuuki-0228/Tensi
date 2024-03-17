#include "error.h"
#include "ConstCreate.h"
#include "SaveCreate.h"
#include "MDataCreate.h"

namespace {
	int NormalEnd() {
		fprintf( stderr, "1>�R�[�h�쐬���I�����܂����B\n" );

		ConstCreate::result();
		SaveCreate::result();
		MDataCreate::result();

		fprintf( stderr, "========== �R�[�h�쐬: 1 ����I���A0 ���s ==========\n" );
		return S_OK;
	}
	int ErrorEnd() {
		fprintf( stderr, "1>�R�[�h�쐬�Ɏ��s���܂����B\n" );
		fprintf( stderr, "========== �R�[�h�쐬: 0 ����I���A1 ���s ==========\n" );
		return E_FAIL;
	}
}

int main() {
	fprintf( stderr, "Creator���J�n���܂���...\n" );
	fprintf( stderr, "1>------ Creator �J�n: �v���O�C��: creator ------\n" );
	fprintf( stderr, "1>�R�[�h�쐬���Ă��܂��B\n" );

	if ( FAILED( ConstCreate::main()	) ) return ErrorEnd();
	if ( FAILED( SaveCreate::main()		) ) return ErrorEnd();
	if ( FAILED( MDataCreate::main()	) ) return ErrorEnd();

	return NormalEnd();
}