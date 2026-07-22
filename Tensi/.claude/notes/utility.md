# Utility フォルダ概要

このファイルは Claude Code（このリポジトリで作業するAI）向けの開発メモです。`SourceCode\Utility` に何があるか・自動生成の仕組みで何に気をつけるべきかをまとめています。
CLAUDE.md には入れていません（毎回読む内容ではないため）。必要になったとき（新しい機能を作る前に既存のものが無いか確認したいとき、Const 周りをいじるときなど）にこのファイルを開いてください。

---

## 1. 定数などの自動生成（creator）について

`Library\creator\creator.exe` を実行すると、以下のファイルが**まるごと再生成**されます。

- 定数プログラム：`SourceCode\Utility\Const\Const.h` / `Const.cpp`（元データ：`Data\Parameter\Const\*.json`）
- セーブプログラム：`CommonSaveData.h` / `.cpp`
- マスターデータプログラム：`MasterData.h` / `.cpp`
- シーンプログラムの雛形（`Scene\Scenes.h` の宣言を見て、無ければ最低限のファイルを作成）

導入手順・外部ツール登録などの詳細は [`Library\creator\README.md`](../../Library/creator/README.md) を参照。

### 気をつけること（実装時に踏んだ地雷）

- **通常の Debug / Release ビルドでは creator は自動実行されない。**
  `Tensi.vcxproj` の Debug/Release 構成の `PreBuildEvent` は空。creator が走るのは専用の `Creator` 構成（NMake）を選んでビルドした時か、`creator.exe` を手動実行した時だけ。
  → `Data\Parameter\Const\*.json` を追加しただけでは `Const.h`/`.cpp` に反映されない。反映させるには creator を実行するか、同じ形式で `Const.h`/`.cpp` に手で構造体を追加する（後で creator を回せば同じ内容が再生成されるので実害はない）。
- **JSON の書式**：`"KEY": ["コメント", 値]`。値が配列だと `std::vector<T>` になる。2〜4要素かつ全部 float の配列は `D3DXVECTOR2/3/4` に、それ以外の2要素配列は `std::pair<A,B>`（コメントに `<A>` `<B>` のように山括弧2組を含めておくと pair 扱いになる仕組み）になる。
- **フィールドの並び順**：JSON のキー順ではなく、内部の json ライブラリの仕様でアルファベット順になる。
- **文字コード**：`Const.h`/`.cpp` 自体は Shift-JIS(CP932)。`Data\Parameter\Const\*.json` は UTF-8（BOM無し）。

### WinSock2 を使うファイルでの include 順の注意

`Const.h` → `Global.h` → `windows.h` という依存があり、`windows.h` は（先に `WinSock2.h` を include していないと）旧い `winsock.h`(v1) を引き込んでしまう。この状態で後から `<WinSock2.h>` を include すると構造体・関数の多重定義エラーになる。
**WinSock2 を使う .cpp では、`SystemSettings.h` の直後・他の重い include より前に `<WinSock2.h>` / `<WS2tcpip.h>` を書くこと。**（例：`SourceCode\Utility\Network\NetworkManager.cpp` 冒頭）

---

## 2. Utility 一覧（既にあるものを重複実装しないためのカタログ）

多くは `SystemSettings.h` の `#define ENABLE_XXX` で有効/無効を切り替えられる（コメントアウトで無効化）。実装前に該当フラグが無効化されていないか確認すること。

### 通信・非同期処理

| フォルダ | 概要 | フラグ |
|---|---|---|
| `Network` | ローカル通信(同一ネットワーク)とネットワーク通信(ホスト/参加形式)。TCP+UDP、部屋番号、UPnPポート開放、状態自動同期、任意メッセージ送受信 | `ENABLE_NETWORK` |
| `ThreadManager` | ワーカースレッドプールへのジョブ投入。結果受け取り・順序保証・専用スレッド・メインスレッドへのコールバック戻しに対応 | `ENABLE_THREAD` |
| `WindowsMessageBox` | Windows のメッセージボックス（はい/いいえ、OK のみ）。`YesNoAsync` は別スレッド表示＋`ThreadManager::Update` でコールバック | フラグ無し（常時） |

### データ管理・定数

| フォルダ | 概要 | フラグ |
|---|---|---|
| `Const` | JSON 駆動の定数クラス。**creator による自動生成**（上記1章参照） | `ENABLE_CONST` |
| `MasterData` | マスターデータ（JSON等から読み込む固定データ）へのアクセス。`MasterDatas.h` と併せて creator が生成 | `ENABLE_MASTER_DATA` |
| `SaveDataManager` | セーブデータ管理。`CommonSaveData`（creator生成）、`FlowerSaveData`、`WeedSaveData` | フラグ無し（`CommonSaveData` は別途） |
| `CashManager` | キャッシュ（一時データ）マネージャー。`std::any` ベース | `ENABLE_CASH` |
| `FileManager` | ファイル入出力全般。`Json`（JSON読み書き）、`DragAndDrop`（D&D受付）、`ImagSize`（jpg/png/bmpのサイズ取得） | `ENABLE_FILE` |

### イベント・状態管理

| フォルダ | 概要 | フラグ |
|---|---|---|
| `EventSubject` | Observer パターンの実装。`Subject`（`CSubject<T>` テンプレート）+ `EventParam` | `ENABLE_OBSERVER` |
| `Bool` | `CBool`（オリジナル bool クラス）。`BitFlagManager`（uint16_t 等をビットフラグ管理するハンドル方式）で一括管理。`DebugFlagWindow` はデバッグ表示 | `ENABLE_CLASS_BOOL` |

### Windows 連携

| フォルダ | 概要 | フラグ |
|---|---|---|
| `WindowsWindowManager` | 他アプリのウィンドウ情報取得・操作（アイコン矩形取得など。マルチモニタ座標のズレに注意 → メモリ `desktop-icon-rect-coordinate-offset` 参照） | `ENABLE_WINDOWS_WINDOW` |
| `WindowsMenuManager` | システムメニュー（タスクトレイ右クリックメニュー等）の管理 | `ENABLE_WINDOWS_MENU` |
| `WindowsShortCutManager` | ショートカット(.lnk)ファイルの作成、スタートアップ登録 | `ENABLE_WINDOWS_SHORT_CUT` |
| `WindowsMenuManager`（タスクトレイ関連） | タスクトレイ機能 | `ENABLE_WINDOWS_TASK_TRAY`（SystemSettings.h 参照） |

### 入力

| フォルダ | 概要 |
|---|---|
| `Input` | 入力全般の窓口。`Key\KeyInput`、`Key\DebugKeyInput`、`KeyLog`、`Controller\XInput`、`Controller\DirectInput`、`KeyBind`（キーバインド設定）、`MouseSpeedGuard`（クラッシュ・強制終了時でもマウス速度を元に戻す監視プロセス機構） |

### アニメーション

| フォルダ | 概要 |
|---|---|
| `Animation\*` | `FadeAnimation` `SlideAnimation` `ShakeAnimation` `BlinkAnimation` `PulseAnimation` `ButtonAnimation` `PopupAnimation` `FloatingAnimation` `SpringAnimation` 等、UI/オブジェクト向けの汎用アニメーション一式 | `ENABLE_ANIMATION` |
| `Easing` | イージング関数 | `ENABLE_EASING` |

### デバッグ・UI

| フォルダ | 概要 |
|---|---|
| `ImGuiManager` | ImGui の初期化・管理。`DebugWindow`、`MessageWindow` を内包 |

### その他（数学・汎用）

| フォルダ | 概要 | フラグ |
|---|---|---|
| `Math` | 数式関連 | `ENABLE_MATH` |
| `Physics` | 物理挙動関連 | `ENABLE_PHYSICS` |
| `Color` | 色関連 | `ENABLE_COLOR` |
| `Random` | 乱数 | フラグ無し |
| `StringConversion` | 文字コード変換（Shift-JIS ⇔ UTF-8 ⇔ wstring 等）。Const/MasterData の JSON 読み込みでも使用 | `ENABLE_STRING_CONVERSION` |
| `TimeManager` | 時間管理 | `ENABLE_TIME` |
| `FrameRate` | FPS制御・待機処理 | フラグ無し |
| `Log` | ログ出力（`Data\$system.log`、CP932） | フラグ無し |
| `Transform` | トランスフォーム。`PositionRenderer`（3D座標のデバッグ表示、`ENABLE_MESH` 配下） | フラグ無し（一部 `ENABLE_MESH`） |

---

## 3. 更新履歴

- 2026-07-23: Network（通信）機能追加時に本ファイルを新規作成（当初 `SourceCode\Utility\README.md`）。creator の PreBuild 未接続・include 順の地雷を記録。
- 2026-07-23: `.claude\notes\utility.md` へ移動（ソースツリーではなく Claude Code 専用の作業メモとして管理するため）。
