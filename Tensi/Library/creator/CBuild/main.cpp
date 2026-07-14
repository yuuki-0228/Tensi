#include "error.h"
#include "ConstCreate.h"
#include "SaveCreate.h"
#include "MDataCreate.h"
#include <exception>
#include <stdexcept>

namespace {
	int NormalEnd() {
		fprintf( stderr, "1>コード作成が終了しました。\n" );

		ConstCreate::result();
		SaveCreate::result();
		MDataCreate::result();

		fprintf( stderr, "========== コード作成: 1 正常終了、0 失敗 ==========\n" );
		return S_OK;
	}
	int ErrorEnd() {
		fprintf( stderr, "1>コード作成に失敗しました。\n" );
		fprintf( stderr, "========== コード作成: 0 正常終了、1 失敗 ==========\n" );
		return E_FAIL;
	}
}

int main() {
	fprintf( stderr, "Creatorを開始しました...\n" );
	fprintf( stderr, "1>------ Creator 開始: プラグイン: creator ------\n" );
	fprintf( stderr, "1>コード作成しています。\n" );

	try {
		if ( FAILED( ConstCreate::main()	) ) return ErrorEnd();
		if ( FAILED( SaveCreate::main()		) ) return ErrorEnd();
		if ( FAILED( MDataCreate::main()	) ) return ErrorEnd();
	}
	catch ( const std::exception& e ) {
		fprintf( stderr, std::string( std::string( "1>" ) + e.what() + std::string( "\n" ) ).c_str() );
		return ErrorEnd();
	}

	return NormalEnd();
}