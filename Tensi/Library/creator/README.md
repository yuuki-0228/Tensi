//-----------------------------------------  
// creatorプラグインについて  
//-----------------------------------------  
■ 行ってくれること  
以下のファイルを自動で作成してくれます  
・定数プログラム（Const.h / Const.cpp）  
・セーブプログラム（CommonSaveData.h / CommonSaveData.cpp）  
・マスターデータプログラム（MasterData.h / MasterData.cpp）  
・シーンプログラム（Scene/Scenes.h を参照）  
　シーンの宣言（None、Max 以外のコメントで囲まれているエリア）を読み込み  
　以下がなければ最低限の内容で作成します  
　　・SourceCode\Scene\シーン名\シーン名.h / .cpp（CSceneBase を継承）  
　　・SourceCode\Object\GameObject\Widget\SceneWidget\シーン名Widget\シーン名Widget.h / .cpp（CSceneWidget を継承）  
　　・SourceCode\Object\GameObject\Widget\UIWidget\シーン名 ディレクトリ  
　また、シーンの宣言に存在しないディレクトリがあれば最後に報告します（削除などはしません）  
　※作成されたファイルの Tensi.vcxproj への追加（VS のプロジェクトへの登録）は手動で行ってください  
  
■設定方法  
・プラグイン導入  
　このプラグインを「Library」フォルダの中に入れてください  
　例）Test.sln  
　　Desktop\Test\Test\Library\creator  
・プラグイン設定  
　A：ビルド時に実行  
　　ソリューションエクスプローラーでプロジェクトのプロパティを開く  
　　ビルドイベントのビルド前のイベントに  
　　　・コマンドライン  
　　　　call Library\creator\creator.exe  
　　また設定時  
　　構成：Debug、プラットフォーム：すべてのプラットフォーム  
　　になっていることを確認してください  
　B：ツールにボタンを追加  
　　[ツール]から[外部ツール]をクリック  
　　[追加]を押し以下の設定をする（一番上に配置する）  
　　　・タイトル  
　　　　creator  
　　　・コマンド  
　　　　「プロジェクト名」\Library\creator\creator.exe  
　　　・初期ディレクトリ  
　　　　$(ProjectDir)  
　　　・出力ウィンドウを使用にチェックを入れる  
　　[ツール]から[カスタマイズ]をクリック  
　　コマンドに移動  
　　ツールバーを選択し「標準」を選ぶ  
　　[コマンドの追加]から[ツール]、「外部コマンド1」を選択  
　　好きな位置に移動（デバッグターゲットの下あたりがおすすめ）  
  
■細かな仕様  
ファイルパスの変更は  
creator\main.cpp  
creator\Create.cpp  
に設定されているファイルパスを変更してください  
  
またメインプログラムによっては自動で生成されたプログラムが  
動かない可能性があります  
  
■ビルド時に行う内容の変更  
creatorにあるCBuild.sln(vs2019)  
を起動し編集してください  
編集後releaseでビルドしてください  
  
確認はdebugのみになります（releaseだと正常に動作しません）  
  
debug確認時に作成される  
「x64」ファイルは削除しても問題ありません  
・削除可能ファイル  
　creator\x64  
　creator\CBuild\x64  
　creator\.vs  