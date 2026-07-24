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

### 描画・視覚効果

| フォルダ | 概要 | フラグ |
|---|---|---|
| `VisualEffects` | ポストエフェクト集（全40種）。色調系：白黒/セピア/色反転/ポスタライズ/カラーフィルター/色調補正/ビネット/2値化/色相シフト。質感系：ノイズ/フィルムグレイン/スキャンライン/CRT/ピクセル化/RGB分離/グリッチ/オールドフィルム。アート系：トゥーン/水彩/油絵/色鉛筆/ハーフトーン/エンボス。エッジ系：アウトライン/エッジ検出/エッジ強調/シャープ。ブラー系：ガウス/放射/モーション/ソフトフォーカス/ブルーム/曇り。歪み系：波/魚眼/熱気揺らぎ。天候系：水滴/雨/雪/フォグ(ミスト式)/深度フォグ。`VisualEffectManager::AddScreenEffect` で画面全体、`BeginCapture`/`EndCapture` で囲った描画物のみ（2D/3D不問）に適用。`Param<SXXXParam>()` でパラメータ設定、ImGui の DebugWindow「VisualEffect」タブで実行中に調整可 | `ENABLE_VISUAL_EFFECT` |

#### VisualEffects 実装メモ（地雷含む）

- シェーダは `Data\Shader\VisualEffects\*.hlsl`（CP932）。実行時コンパイルなので **hlsl を書き換えたら再起動だけで反映**される。エントリ追加時は fxc での事前検証が楽（`/T ps_5_0 /E PS_XXX`）
- **重要な地雷（微分と可変フロー）**: このアプリの実行時コンパイラは古い D3DX11(`D3DCompiler_43`)。`Texture2D.Sample()` はミップ選択用の微分を暗黙計算するため、**ピクセル依存の分岐（`if(IsOutArea(...)) return;` の後や、`continue` を含むループ／早期returnを持つ関数の中）で、計算した座標を `Sample()` すると `error X8000: ... derivatives ... within flow control that could vary across pixels` でコンパイル失敗**する。失敗したエフェクトはチェーンで無言スキップされ「何も表示されない」ようになる（fxc(d3dcompiler_47)は通ってしまうので気付きにくい）。**対策: 分岐後に計算座標をサンプルする箇所は `SampleLevel(sampler, uv, 0.0f)`（微分不要）を使う**。描画先RTはミップ1枚なので結果は同一。Raindrop/Rain の GlassBlur・レンズ・水筋はこれで対応済み。関数先頭の `Sample(i.UV)`（分岐前・生の補間座標）は従来 `Sample` のままでよい。**新しいポストエフェクトを足すときは、IsOutArea の return より後ろのテクスチャ読みは SampleLevel を使うこと**
- コンパイル失敗の切り分けは `Data\$system.log`（各起動で新規作成・CP932）を `CVisualEffectBase::CompilePS` で grep。「Invert は効くが特定エフェクトだけ無表示」はこのコンパイル失敗を疑う
- 画面全体適用は `Main.cpp` が `SceneManager::Render()` 直後に `ApplyScreenEffects()` を自動で呼ぶ。**UI を対象外にしたい場合はシーン側で UI 描画前に手動で呼ぶ**（そのフレームの自動適用はスキップされる）
- MSAA 有効時はフレーム途中で `m_pSceneTex` を Resolve してエフェクトを掛け、結果を MSAA シーンRTV に書き戻す（Present の既存 Resolve はそのまま活きる）。このために `DirectX11` に `GetMsaaSampleCount / GetSceneTex / GetSceneRTV / GetBackBufferRTV / GetBackBufferTex` を追加した
- ラスタライザステートは `DirectX11` のキャッシュとずれないように**自前ステートを RSSetState 直接**で設定し、`RSGetState` で取った元ポインタへ戻している。ブレンド/深度は getter で退避して setter で復元（`SetAlphaBlend` は AlphaToCoverage 有効中は無視される点に注意）
- 透明ウィンドウ（本プロジェクト）ではアルファ0の部分にエフェクトは見えない。`SetAlphaLift` で持ち上げると見えるが**クリック透過判定にも影響**する。通常ウィンドウの別プロジェクトでは気にしなくてよい
- キャプチャ合成は「透明クリア→描画→ストレートアルファ合成」なので、半透明の縁にわずかな黒フリンジが出ることがある
- **フォグは2種類**：`Fog`（ノイズミスト式・深度不要・2D/3D両用）と `FogDepth`（深度バッファ参照・3D向け）。`FogDepth` の Near/Far はカメラのニア/ファーと手動で合わせる（カメラ側に getter が無いため）。旧 `Common\Fog`（メッシュ向け・未使用）はこの移行時に削除済み（`ENABLE_FOG` フラグも撤去）
- 深度フォグのために深度バッファは **TYPELESS 形式＋SRV 付き**で作成するよう `DirectX11::CreateDepthStencilBackBufferRTV` を変更した（`GetDepthSRV` で取得。MSAA 時は `Texture2DMS` として参照。非対応環境では従来形式にフォールバックし SRV は nullptr）
- 雨/雪/フォグ/ミスト等のオーバーレイ系は、効果が乗った部分の**アルファも自動で持ち上がる**（透明ウィンドウのデスクトップ上にも見える）。トゥーン等の加工系は元のアルファを維持
- **Rain（雨滴）は BigWIngs「Heartfelt / Rain on Screen」系の手続き的雨ガラス**（`Weather.hlsl` の `DropLayer`/`RainHeight`/`PS_Rain`）。列を流れ落ちる水滴＋背後に残る雫のトレイルを**高さ場**として生成し、その勾配を法線として、ぼかしたガラス面（`GlassBlur`）を屈折させてサンプリングする。**水滴はすべて流れ落ちる（静止した水玉は無い）**。水のある所ほどぼかしを弱めて鮮明（水滴がレンズになる）。パラメータ：`Amount`（雨量）/`Speed`（落下速度）/`Refract`（屈折）/`Blur`（すりガラスぼかし）/`Brightness`（ハイライト）。参照：ユーザ指定の noriben327 の Unity 実装（BigWIngs ShaderToy "Heartfelt" 相当）
- 法線は**高さ場を3点評価した手動の有限差分**で求めている（`ddx/ddy` を使わない＝上記の微分ハマりを回避）。透明背景では屈折対象が無いので縁＋ハイライト＋アルファで薄く見える程度。**本領は不透明背景のゲーム**で発揮（ユーザの想定用途）
- **旧 Raindrop / 旧 Rain(水筋版) は削除して Rain に統一**（2026-07-24）：当初は「Raindrop＝ビーズ」「Rain＝リビュレット(縦の水筋)」の2種だったが、水筋版が破綻していた・見た目が近い等の理由で、良い方（ビーズ）を `EVisualEffect::Rain` に一本化し `Raindrop` を撤去。`EVisualEffect` の enum から `Raindrop` を削除したので、以降 enum 値の並びが変わっている点に注意（`EFFECT_NAMES` 配列と `CreateEffect` の並びも合わせてある。enum要素数＝EFFECT_NAMES要素数＝40）
- 油絵（Kuwahara フィルタ）は Radius に応じてサンプル数が増える（Radius 6 で約200タップ）。全画面常用は Radius 3〜4 までが無難

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
- 2026-07-24: `VisualEffects`（ポストエフェクト22種）を追加。併せて、コミット bfaf45c で `Utility\Message` が削除されたのに `Main.cpp` / `Sprite.cpp` / `SystemWindowManager.cpp` に include が残っていてビルドが壊れていたため、stale include を除去した（`ErrorMessage` 等は `Global.h` に定義済み）。
- 2026-07-24: アート系・エッジ系・天候系など19種を追加し全41種に。`Common\Fog` を廃止して `Fog`（ミスト式）/`FogDepth`（深度参照）へ移行。深度バッファを SRV 対応（TYPELESS化）に変更。
