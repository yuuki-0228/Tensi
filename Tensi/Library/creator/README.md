//-----------------------------------------
// creatorプラグインについて
//-----------------------------------------
■ 行ってくれること
デバックビルド前に特定のファイルを自動で作成してくれます
・定数プログラム（Const.h / Const.cpp）
・セーブプログラム（CommonSaveData.h / CommonSaveData.cpp）
・マスターデータプログラム（MasterData.h / MasterData.cpp）

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