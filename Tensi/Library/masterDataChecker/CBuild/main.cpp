#include "error.h"
#include "Check.h"

namespace {
	int NormalEnd() {
		fprintf( stderr, "1>コード作成が終了しました。\n" );

		Check::result();

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
	fprintf( stderr, "MasterDataCheckerを開始しました...\n" );
	fprintf( stderr, "1>------ MasterDataChecker 開始: プラグイン: masterDataChecker ------\n" );
	fprintf( stderr, "1>コード作成しています。\n" );

	if ( FAILED( Check::main()	) ) return ErrorEnd();

	return NormalEnd();
}