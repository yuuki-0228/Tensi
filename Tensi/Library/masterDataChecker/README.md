//-----------------------------------------  
// MasterDataCheckerプラグインについて  
//-----------------------------------------  
■ 行ってくれること  
マスターデータを確認してくれます  
  
■設定方法  
・プラグイン導入  
　このプラグインを「Library」フォルダの中に入れてください  
　例）Test.sln  
　　Desktop\Test\Test\Library\masterDataChecker  
・プラグイン設定  
　ソリューションエクスプローラーでプロジェクトのプロパティを開く  
　ビルドイベントのビルド前のイベントに  
　　・コマンドライン  
　　　call Library\masterDataChecker\masterDataChecker.exe  
　また設定時  
　構成：Debug、プラットフォーム：すべてのプラットフォーム  
　になっていることを確認してください  
  
■細かな仕様  
ファイルパスの変更は  
masterDataChecker\main.cpp  
に設定されているファイルパスを変更してください  
  
またメインプログラムによっては自動で生成されたプログラムが  
動かない可能性があります  
  
■ビルド時に行う内容の変更  
masterDataCheckerにあるCBuild.sln(vs2019)  
を起動し編集してください  
編集後releaseでビルドしてください  
  
確認はdebugのみになります（releaseだと正常に動作しません）  
  
debug確認時に作成される  
「x64」ファイルは削除しても問題ありません  
・削除可能ファイル  
　masterDataChecker\x64  
　masterDataChecker\CBuild\x64  
　masterDataChecker\.vs  