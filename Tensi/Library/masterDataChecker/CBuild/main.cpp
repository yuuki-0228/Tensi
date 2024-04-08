#include "error.h"
#include "Check.h"

namespace {
	int NormalEnd() {
		fprintf( stderr, "1>�R�[�h�쐬���I�����܂����B\n" );

		Check::result();

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
	fprintf( stderr, "MasterDataChecker���J�n���܂���...\n" );
	fprintf( stderr, "1>------ MasterDataChecker �J�n: �v���O�C��: masterDataChecker ------\n" );
	fprintf( stderr, "1>�R�[�h�쐬���Ă��܂��B\n" );

	if ( FAILED( Check::main()	) ) return ErrorEnd();

	return NormalEnd();
}