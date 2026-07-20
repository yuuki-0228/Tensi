#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_THREAD
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>

/************************************************
*	スレッドマネージャー.
*	複数個所から安全にスレッド処理を投げられるようにする.
*
*	使い方.
*	・短時間の処理 : Start( 処理 ) … 固定数のワーカースレッドが順次実行する.
*	・結果を受け取る処理 : StartResult( 処理, 受け取り関数 ) … 処理の戻り値をメインスレッドで受け取る.
*	・順番を守りたい処理 : StartSequential( タグ, 処理 ) … 同じタグ同士は同時実行されず投入順に実行される.
*	・監視ループなど長時間動く処理 : StartDedicated( 処理 ) … 専用スレッドを1本立てて実行する.
*	・完了確認 : 戻り値のハンドルを IsFinished / Wait に渡す.
*	・完了後の処理 : 第2引数 OnFinish はメインスレッド( Update 内 )で実行される.
*	・別スレッドからメインスレッドへ処理を送る : RunOnMainThread( 処理 ).
*
*	注意.
*	・ジョブの中から Wait / WaitAll を呼ばないこと( デッドロックの恐れがある ).
*	・専用スレッドの処理は IsStopRequested が true になったら速やかに抜けること.
*	・Release 時、受付済みのジョブは全て実行されてから停止する( 未実行の完了コールバックは破棄される ).
**/
class ThreadManager final
{
public:
	// ジョブの進行状態( 内部用 ).
	struct JobState;

	// ジョブハンドル.
	using JobHandle			= std::shared_ptr<JobState>;
	// ワーカースレッドで実行する処理.
	using JobFunc			= std::function<void()>;
	// 専用スレッドで実行する処理.
	using DedicatedJobFunc	= std::function<void( const std::atomic<bool>& IsStopRequested )>;
	// 完了時にメインスレッドで実行する処理.
	using FinishFunc		= std::function<void()>;

public:
	ThreadManager();
	~ThreadManager();

	// ジョブの追加.
	//	空いているワーカースレッドが順次実行する( 全ワーカー使用中の場合は空き待ちになる ).
	static JobHandle Start( JobFunc Job, FinishFunc OnFinish = nullptr );

	// 結果付きジョブの追加.
	//	戻り値のある処理をワーカースレッドで実行し、その結果を完了時にメインスレッドで受け取る.
	//	結果の型はデフォルト構築できること.
	//	使用例 : ThreadManager::StartResult(
	//				[](){ return HeavyCalc(); },
	//				[]( const ULONGLONG& Result ){ /* メインスレッドで結果を使う */ } );
	template<class TJob, class TFinish>
	static JobHandle StartResult( TJob&& Job, TFinish&& OnFinish )
	{
		using TResult = std::invoke_result_t<TJob>;
		auto pResult = std::make_shared<TResult>();
		return Start(
			[pResult, DoJob = std::forward<TJob>( Job )]() { *pResult = DoJob(); },
			[pResult, DoFinish = std::forward<TFinish>( OnFinish )]() { DoFinish( *pResult ); } );
	}

	// 直列ジョブの追加.
	//	同じタグのジョブは同時実行されず、投入した順番どおりに1つずつ実行される.
	//	( セーブデータやログなど、同じファイルへの書き込みを安全に非同期化する用途 ).
	//	別タグ同士は並列に実行される.
	static JobHandle StartSequential( const std::string& Tag, JobFunc Job, FinishFunc OnFinish = nullptr );

	// 専用スレッドでジョブを実行.
	//	監視ループなど動き続ける処理はこちらを使う( ワーカースレッドを占有しないため ).
	static JobHandle StartDedicated( DedicatedJobFunc Job, FinishFunc OnFinish = nullptr );

	// メインスレッドで実行する処理を積む.
	//	どのスレッドからでも呼べる. 次の Update 時にメインスレッドで実行される.
	static void RunOnMainThread( FinishFunc Func );

	// ジョブを受け付けられるか( Release 後は false ).
	//	false の場合は呼び出し側で同期処理に切り替えること.
	static bool GetIsAvailable();

	// ジョブが終了したか.
	static bool IsFinished( const JobHandle& Handle );

	// ジョブが終了するまで待機する.
	//	ジョブの中から呼ばないこと( デッドロックの恐れがある ).
	static void Wait( const JobHandle& Handle );

	// 全ジョブ( 実行待ち+実行中 )が終了するまで待機する.
	//	専用スレッドと完了コールバックは対象外. ジョブの中から呼ばないこと.
	static void WaitAll();

	// ジョブへ停止要求を送る( 待機はしない ).
	//	専用スレッド : 処理側が IsStopRequested を確認してループを抜ける.
	//	未実行のジョブ : 実行されずに終了扱いになる.
	static void RequestStop( const JobHandle& Handle );

	// 未完了のジョブ数( 実行待ち+実行中 )の取得. 専用スレッドは含まない.
	static int GetActiveJobNum();

	// ワーカースレッド数の取得( 未使用の場合は0 ).
	static int GetWorkerNum();

	// 更新処理.
	//	完了コールバックの実行と終了済み専用スレッドの回収を行うため、メインスレッドで毎フレーム呼ぶ.
	static void Update();

	// 終了処理.
	//	受付済みのジョブを全て実行し、全スレッドの終了を待つ( アプリ終了時に呼ぶ ).
	static void Release();

private:
	// ワーカースレッドで実行するジョブ.
	struct PoolJob
	{
		JobHandle	pState	= nullptr;	// 進行状態.
		JobFunc		Func	= nullptr;	// 実行する処理.
	};
	// 専用スレッド.
	struct DedicatedThread
	{
		JobHandle	pState;	// 進行状態.
		std::thread	Thread;	// スレッド本体.
	};
	// 直列実行のチャンネル( タグごとに1つ ).
	struct SequentialChannel
	{
		std::deque<PoolJob>	Pending;			// 実行待ちのジョブ.
		bool				IsRunning = false;	// 同タグのジョブが実行中か.
	};

private:
	// インスタンスの取得.
	static ThreadManager* GetInstance();

	// ワーカースレッドの作成( 初回使用時に呼ばれる ).
	void CreateWorkers();
	// ワーカースレッドの処理.
	void WorkerLoop();
	// ジョブをプールキューへ積む( 受け付けない場合は false ).
	//	IsForce : 終了中でも積む( 受付済みの直列ジョブの続きを実行しきる用途 ).
	bool PushPoolJob( PoolJob&& Job, const bool IsForce = false );
	// ジョブの終了処理( 終了通知と完了コールバックの積み込み ).
	void FinishJob( const JobHandle& pState );
	// 直列ジョブ終了時の処理( 同タグの次のジョブを実行キューへ積む ).
	void OnSequentialJobFinished( const std::string& Tag );
	// 未完了ジョブ数の加算( 負数で減算. 0になったら WaitAll を起こす ).
	void AddActiveJobNum( const int Add );
	// 全スレッドの停止.
	void Shutdown();

private:
	std::vector<std::thread>		m_Workers;			// ワーカースレッド.
	std::queue<PoolJob>				m_JobQueue;			// 実行待ちのジョブ( m_QueueMutex で保護 ).
	std::mutex						m_QueueMutex;		// ジョブキュー用ミューテックス.
	std::condition_variable			m_QueueCv;			// ジョブ追加/終了要求の通知.
	std::atomic<bool>				m_IsShutdown;		// 終了中か.

	std::unordered_map<std::string, SequentialChannel>	m_Channels;	// 直列実行のチャンネル( m_ChannelMutex で保護 ).
	std::mutex						m_ChannelMutex;		// 直列実行チャンネル用ミューテックス.

	std::vector<DedicatedThread>	m_DedicatedThreads;	// 専用スレッド( m_DedicatedMutex で保護 ).
	std::mutex						m_DedicatedMutex;	// 専用スレッド用ミューテックス.

	std::vector<FinishFunc>			m_FinishFuncs;		// メインスレッドで実行する完了コールバック( m_FinishMutex で保護 ).
	std::mutex						m_FinishMutex;		// 完了コールバック用ミューテックス.

	int								m_ActiveJobNum;		// 未完了のジョブ数( m_ActiveMutex で保護 ).
	std::mutex						m_ActiveMutex;		// 未完了ジョブ数用ミューテックス.
	std::condition_variable			m_ActiveCv;			// 全ジョブ完了の通知.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	ThreadManager( const ThreadManager& )				= delete;
	ThreadManager& operator = ( const ThreadManager& )	= delete;
	ThreadManager( ThreadManager&& )					= delete;
	ThreadManager& operator = ( ThreadManager&& )		= delete;
};

#endif	// #ifdef ENABLE_THREAD.
