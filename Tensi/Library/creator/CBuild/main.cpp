#include "error.h"
#include "ConstCreate.h"
#include "SaveCreate.h"
#include "MDataCreate.h"
#include "SceneCreate.h"
#include <exception>
#include <stdexcept>

namespace {
	int NormalEnd() {
		PrintOutput( "1>コード作成が終了しました。\n" );

		ConstCreate::result();
		SaveCreate::result();
		MDataCreate::result();
		SceneCreate::result();

		PrintOutput( "========== コード作成: 1 正常終了、0 失敗 ==========\n" );
		return S_OK;
	}
	int ErrorEnd() {
		PrintOutput( "1>コード作成に失敗しました。\n" );
		PrintOutput( "========== コード作成: 0 正常終了、1 失敗 ==========\n" );
		return E_FAIL;
	}
}

int main() {
	PrintOutput( "Creatorを開始しました...\n" );
	PrintOutput( "1>------ Creator 開始: プラグイン: creator ------\n" );
	PrintOutput( "1>コード作成しています。\n" );

	try {
		if ( FAILED( ConstCreate::main()	) ) return ErrorEnd();
		if ( FAILED( SaveCreate::main()		) ) return ErrorEnd();
		if ( FAILED( MDataCreate::main()	) ) return ErrorEnd();
		if ( FAILED( SceneCreate::main()	) ) return ErrorEnd();
	}
	catch ( const std::exception& e ) {
		PrintOutput( std::string( "1>" ) + e.what() + std::string( "\n" ) );
		return ErrorEnd();
	}

	return NormalEnd();
}