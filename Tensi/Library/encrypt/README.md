//-----------------------------------------  
// encryptプラグインについて  
//-----------------------------------------  
■ 行ってくれること  
暗号化フォルダ（RData）を自動で作成してくれます  
Data直下のフォルダごとに暗号化して1つのアーカイブ（フォルダ名.gda）にまとめます  
例）Data\Sprite → RData\Sprite.gda  
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

■アーカイブ除外リスト（remove_file.txt）について  
encrypt\remove_file.txt  
に書いてあるフォルダ・ファイルはアーカイブせず  
そのまま「RData」にコピーされます  
（実行時に書き込みが発生するフォルダや、実フォルダとして必要なものを書いてください）  
・書き方：「,」または改行区切りで名前を書く（Data直下の名前）  
・文字コードはUTF-8とShift-JISのどちらでも読み込めます  

また、以下はリストに書かなくても自動で処理されます  
・Data直下のファイル（Icon.ico、imgui.ini、$system.logなど）はアーカイブせずコピー  
・「Data\Debug」フォルダは開発専用のためRDataに含めない  
・空フォルダはアーカイブを作成しない  

■暗号化の復元について  
ソリューションエクスプローラーで  
Library\encrypt\key.h  
Library\encrypt\key.cpp  
Library\encrypt\file.h  
Library\encrypt\file.cpp  
をプロジェクトに含めてください  
「encrypt\file」にある  
「GetRestoreFile()」を使用することでファイルのバッファを取得できます  
　＞実ファイルが無い場合は自動でアーカイブ（.gda）の中から取得します  
　＞実ファイルがある場合はそちらを優先します（ルーズファイル優先）  
　＞暗号化ファイル（#○○.bin）の場合のみ復元処理を行います  
　＞std::istreamに変換する際は「encrypt::membuf」を使用してください  
「EnumerateDataFiles()」を使用することでフォルダ内のファイルを列挙できます  
　＞実フォルダとアーカイブの両方から列挙します  
　＞std::filesystem::recursive_directory_iteratorの代わりに使用してください  
「GetEncryptionFilePath()」を使用することで暗号化したファイルの場合、  
　対応したファイルパスに変換してくれます  
「GetIsEncryption()」を使用することで暗号化しているファイルか取得できます  
「GetIsArchiveFile()」を使用することでアーカイブ内にあるファイルか取得できます  

例）  
#include <encrypt/file.h>  

auto rf = encrypt::GetRestoreFile( "Data\\#hoge10.bin" );  
encrypt::membuf mb( rf );  
std::istream i( &mb );  

■細かな仕様  
ビルド時にDataフォルダの中にあるファイルを暗号化しアーカイブにします  
また、暗号化するファイルの種類に関しては  
encrypt\key.h  
にある「FILE_LIST」で設定することができます  

暗号化したファイルには"#"が付きます  
（復元する際先頭に"#"があれば復元します）  

ファイル名の最後に特定のIDが付きます  
#hoge「10」.bin  
このIDは何のファイルだったかを示しています  
IDについては encrypt\key.h を確認してください  

暗号化対象外のファイル（binなど）もアーカイブには入ります  
（復元処理は行わずそのまま取得できます）  

■アーカイブ（.gda）のファイル構造  
[マジックナンバー "GDA1" 4byte]  
[エントリ数 4byte]  
[インデックス：エントリごとに  
　パス長(4byte) / パス(暗号化済み) / データ位置(8byte) / データサイズ(8byte)]  
[データ本体]  
※インデックスのパスはSECRET_KEYで暗号化されています  

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
