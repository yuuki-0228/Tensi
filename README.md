## ビルド構成

| 構成 | 用途 |
|---|---|
| `Debug` | 開発用。ビルド前後イベントは無し（creator・encrypt は動かない） |
| `Release` | リリース用。ビルド後イベントで `Library\encrypt\encrypt.exe` が自動実行され、`Data` フォルダから暗号化アーカイブ `RData` を生成する |
| `Creator` | NMake 構成。ビルドすると `Library\creator\creator.exe` が実行され、`Const.h`/`.cpp` 等の自動生成ファイルを再生成する |

ログは `Data\$system.log` に出力されます。

## プロジェクト構成

```
SourceCode/
  Main/      … エントリポイント（WinMain）
  System/    … システム全般
  Scene/     … シーン管理（SceneBase / SceneManager / FadeManager など）
  Object/    … ゲームオブジェクト・UI ウィジェット
  Resource/  … リソース読み込み関連
  Editor/    … エディタ機能
  Common/    … 共通処理
  Utility/   … 汎用ユーティリティ群（下記参照）
  Global.h          … プロジェクト全体で使うグローバル定義
  SystemSettings.h  … 機能の有効/無効を切り替える ENABLE_XXX 定義集
```

## creator について（定数・セーブ・マスターデータの自動生成）

`Library\creator\creator.exe` を実行すると、以下がまるごと再生成されます。

- 定数プログラム：`Const.h` / `Const.cpp`（元データは `Data\Parameter\Const\*.json`）
- セーブプログラム：`CommonSaveData.h` / `.cpp`
- マスターデータプログラム：`MasterData.h` / `.cpp`
- シーンプログラムの雛形（`Scene\Scenes.h` の宣言を見て、無ければ最低限のファイルを作成。作成されたファイルの `.vcxproj` への追加は手動で行う）

