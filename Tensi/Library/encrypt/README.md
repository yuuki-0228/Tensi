//-----------------------------------------
// encryptプラグインについて
//-----------------------------------------
■ 行ってくれること
リリースビルド後に暗号化フォルダ（RData）を自動で作成してくれます
作成された「RData」をリリース時は「Data」に名前変更し使用するようにしてください
（プロジェクトでファイルの復元処理の対応が必要になります（■復元についてを確認））

■設定方法
・プラグイン導入
　このプラグインを「Library」フォルダの中に入れてください
　例）Test.sln
　　Desktop\Test\Test\Library\encrypt
・プラグイン設定
　ソリューションエクスプローラーでプロジェクトのプロパティを開く
　ビルドイベントのビルド後のイベントに
　　・コマンドライン
　　　call Library\encrypt\encrypt.exe

　を追加してください
　また設定時
　　構成：Relese、プラットフォーム：すべてのプラットフォーム
　になっていることを確認してください

■暗号化で使用する秘密鍵について
encrypt\key.h
にある「SECRET_KEY」
で設定することができます
プロジェクトごとに変更することをお勧めします

■暗号化の復元について
ソリューションエクスプローラーで
Library\encrypt\key.h
Library\encrypt\key.cpp
Library\encrypt\file.h
Library\encrypt\file.cpp
をプロジェクトに含めてください
「encrypt\file」にある
「GetRestoreFile()」を使用することでファイルのバッファを取得できます
　＞std::istreamに変換する際は「encrypt::membuf」を使用してください
「GetEncryptionFilePath()」を使用することで暗号化したファイルの場合、
　対応したファイルパスに変換してくれます
「GetIsEncryption()」を使用することで暗号化しているファイルか取得できます

例）
#include <encrypt/file.h>

auto rf = encrypt::GetRestoreFile( "Data\\#hoge10.bin" );
encrypt::membuf mb( rfd );
std::istream i( &mb );

■細かな仕様
ビルド時にDataフォルダの中にあるファイルを暗号化します
また、暗号化するファイルの種類に関しては
encrypt\key.h
にある「FILE_LIST」で設定することができます

暗号化したファイルには"#"が付きます
（復元する際先頭に"#"があれば復元します）

ファイル名の最後に特定のIDが付きます
#hoge「10」.bin
このIDは何のファイルだったかを示しています
IDについては encrypt\key.h を確認してください

■ビルド時に行う内容の変更
encryptにあるCBuild.sln(vs2019)
を起動し「main.cpp」を編集してください
編集後releaseでビルドしてください

確認はdebugのみになります（releaseだと正常に動作しません）

debug確認時に作成される
「x64」ファイルは削除しても問題ありません
・削除可能ファイル
　encrypt\x64
　encrypt\CBuild\x64
　encrypt\.vs