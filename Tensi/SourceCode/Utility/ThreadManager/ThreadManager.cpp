#include "ThreadManager.h"
#ifdef ENABLE_THREAD
#include "..\Log\Log.h"

//----------------------------.
// ジョブの進行状態.
//----------------------------.
struct ThreadManager::JobState
{
	std::atomic<bool>		IsFinished;			// 終了したか.
	std::atomic<bool>		IsStopRequested;	// 停止要求が来たか.
	FinishFunc				OnFinish;			// 完了時にメインスレッドで実行する処理.
	std::string				Tag;				// 直列実行のタグ.
	bool					IsSequential;		// 直列ジョブか.
	bool					IsDedicated;		// 専用スレッドのジョブか.
	std::mutex				Mutex;				// 終了待機用ミューテックス.
	std::condition_variable	FinishCv;			// 終了通知.

	JobState()
		: IsFinished		( false )
		, IsStopRequested	( false )
		, OnFinish			( nullptr )
		, Tag				()
		, IsSequential		( false )
		, IsDedicated		( false )
	{
	}
};

ThreadManager::ThreadManager()
	: m_Workers				()
	, m_JobQueue			()
	, m_QueueMutex			()
	, m_QueueCv				()
	, m_IsShutdown			( false )
	, m_Channels			()
	, m_ChannelMutex		()
	, m_DedicatedThreads	()
	, m_DedicatedMutex		()
	, m_FinishFuncs			()
	, m_FinishMutex			()
	, m_ActiveJobNum		( 0 )
	, m_ActiveMutex			()
	, m_ActiveCv			()
{
}

ThreadManager::~ThreadManager()
{
	// Release が呼ばれていない場合の保険.
	Shutdown();
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
ThreadManager* ThreadManager::GetInstance()
{
	static std::unique_ptr<ThreadManager> pInstance = std::make_unique<ThreadManager>();
	return pInstance.get();
}

//----------------------------.
// ジョブの追加.
//----------------------------.
ThreadManager::JobHandle ThreadManager::Start( JobFunc Job, FinishFunc OnFinish )
{
	if ( Job == nullptr ) return nullptr;
	ThreadManager* pInstance = GetInstance();

	// ハンドルの作成.
	JobHandle pState	= std::make_shared<JobState>();
	pState->OnFinish	= OnFinish;

	// 先に未完了数を増やしておく( 実行が終わってから増やすと WaitAll がすり抜けるため ).
	pInstance->AddActiveJobNum( 1 );
	if ( pInstance->PushPoolJob( PoolJob{ pState, std::move( Job ) } ) == false ) {
		// 終了中は受け付けない( 終了扱いのハンドルを返す ).
		pInstance->AddActiveJobNum( -1 );
		pState->IsFinished = true;
	}
	return pState;
}

//----------------------------.
// 直列ジョブの追加.
//----------------------------.
ThreadManager::JobHandle ThreadManager::StartSequential( const std::string& Tag, JobFunc Job, FinishFunc OnFinish )
{
	if ( Job == nullptr ) return nullptr;
	ThreadManager* pInstance = GetInstance();

	// ハンドルの作成.
	JobHandle pState		= std::make_shared<JobState>();
	pState->OnFinish		= OnFinish;
	pState->Tag				= Tag;
	pState->IsSequential	= true;

	// 先に未完了数を増やしておく.
	pInstance->AddActiveJobNum( 1 );

	bool IsDispatch = false;
	{
		std::lock_guard<std::mutex> Lock( pInstance->m_ChannelMutex );
		// 終了中は受け付けない( 終了扱いのハンドルを返す ).
		if ( pInstance->m_IsShutdown ) {
			pInstance->AddActiveJobNum( -1 );
			pState->IsFinished = true;
			return pState;
		}
		SequentialChannel& Channel = pInstance->m_Channels[Tag];
		if ( Channel.IsRunning ) {
			// 同タグのジョブが実行中の場合は後ろに並べる( 実行キューへは終了時に積まれる ).
			Channel.Pending.emplace_back( PoolJob{ pState, std::move( Job ) } );
		}
		else {
			Channel.IsRunning = true;
			IsDispatch = true;
		}
	}
	if ( IsDispatch ) {
		if ( pInstance->PushPoolJob( PoolJob{ pState, std::move( Job ) } ) == false ) {
			pInstance->AddActiveJobNum( -1 );
			pState->IsFinished = true;
		}
	}
	return pState;
}

//----------------------------.
// 専用スレッドでジョブを実行.
//----------------------------.
ThreadManager::JobHandle ThreadManager::StartDedicated( DedicatedJobFunc Job, FinishFunc OnFinish )
{
	if ( Job == nullptr ) return nullptr;
	ThreadManager* pInstance = GetInstance();

	// ハンドルの作成.
	JobHandle pState		= std::make_shared<JobState>();
	pState->OnFinish		= OnFinish;
	pState->IsDedicated		= true;

	std::lock_guard<std::mutex> Lock( pInstance->m_DedicatedMutex );
	// 終了中は受け付けない( 終了扱いのハンドルを返す ).
	if ( pInstance->m_IsShutdown ) {
		pState->IsFinished = true;
		return pState;
	}

	// 専用スレッドを立てて実行する.
	DedicatedThread Dedicated;
	Dedicated.pState = pState;
	Dedicated.Thread = std::thread(
		[pInstance, pState, DedicatedJob = std::move( Job )]()
		{
			// 既に停止要求が来ている場合は実行しない.
			if ( pState->IsStopRequested == false ) {
				try {
					DedicatedJob( pState->IsStopRequested );
				}
				catch ( const std::exception& e ) {
					Log::PushLogError( std::string( "専用スレッドで例外が発生 : " ) + e.what() );
				}
				catch ( ... ) {
					Log::PushLogError( "専用スレッドで不明な例外が発生" );
				}
			}
			pInstance->FinishJob( pState );
		} );
	pInstance->m_DedicatedThreads.emplace_back( std::move( Dedicated ) );
	return pState;
}

//----------------------------.
// メインスレッドで実行する処理を積む.
//----------------------------.
void ThreadManager::RunOnMainThread( FinishFunc Func )
{
	if ( Func == nullptr ) return;
	ThreadManager* pInstance = GetInstance();
	if ( pInstance->m_IsShutdown ) return;

	std::lock_guard<std::mutex> Lock( pInstance->m_FinishMutex );
	pInstance->m_FinishFuncs.emplace_back( std::move( Func ) );
}

//----------------------------.
// ジョブを受け付けられるか.
//----------------------------.
bool ThreadManager::GetIsAvailable()
{
	return GetInstance()->m_IsShutdown == false;
}

//----------------------------.
// ジョブが終了したか.
//----------------------------.
bool ThreadManager::IsFinished( const JobHandle& Handle )
{
	if ( Handle == nullptr ) return true;
	return Handle->IsFinished;
}

//----------------------------.
// ジョブが終了するまで待機する.
//----------------------------.
void ThreadManager::Wait( const JobHandle& Handle )
{
	if ( Handle == nullptr ) return;
	std::unique_lock<std::mutex> Lock( Handle->Mutex );
	Handle->FinishCv.wait( Lock, [&Handle]() { return Handle->IsFinished == true; } );
}

//----------------------------.
// 全ジョブが終了するまで待機する.
//----------------------------.
void ThreadManager::WaitAll()
{
	ThreadManager* pInstance = GetInstance();

	std::unique_lock<std::mutex> Lock( pInstance->m_ActiveMutex );
	pInstance->m_ActiveCv.wait( Lock, [pInstance]() { return pInstance->m_ActiveJobNum <= 0; } );
}

//----------------------------.
// ジョブへ停止要求を送る.
//----------------------------.
void ThreadManager::RequestStop( const JobHandle& Handle )
{
	if ( Handle == nullptr ) return;
	Handle->IsStopRequested = true;
}

//----------------------------.
// 未完了のジョブ数の取得.
//----------------------------.
int ThreadManager::GetActiveJobNum()
{
	ThreadManager* pInstance = GetInstance();

	std::lock_guard<std::mutex> Lock( pInstance->m_ActiveMutex );
	return pInstance->m_ActiveJobNum;
}

//----------------------------.
// ワーカースレッド数の取得.
//----------------------------.
int ThreadManager::GetWorkerNum()
{
	ThreadManager* pInstance = GetInstance();

	std::lock_guard<std::mutex> Lock( pInstance->m_QueueMutex );
	return static_cast<int>( pInstance->m_Workers.size() );
}

//----------------------------.
// 更新処理.
//----------------------------.
void ThreadManager::Update()
{
	ThreadManager* pInstance = GetInstance();

	// 完了コールバックをメインスレッドで実行する.
	//	( 実行中に新しいジョブを積めるよう、先にロック内で取り出してから実行する ).
	std::vector<FinishFunc> FinishFuncs;
	{
		std::lock_guard<std::mutex> Lock( pInstance->m_FinishMutex );
		FinishFuncs.swap( pInstance->m_FinishFuncs );
	}
	for ( auto& Func : FinishFuncs ) Func();

	// 終了済みの専用スレッドを回収する.
	std::lock_guard<std::mutex> Lock( pInstance->m_DedicatedMutex );
	for ( auto Itr = pInstance->m_DedicatedThreads.begin(); Itr != pInstance->m_DedicatedThreads.end(); ) {
		if ( Itr->pState->IsFinished && Itr->Thread.joinable() ) {
			Itr->Thread.join();
			Itr = pInstance->m_DedicatedThreads.erase( Itr );
		}
		else {
			++Itr;
		}
	}
}

//----------------------------.
// 終了処理.
//----------------------------.
void ThreadManager::Release()
{
	GetInstance()->Shutdown();
}

//----------------------------.
// ワーカースレッドの作成.
//	m_QueueMutex をロックした状態で呼ぶこと.
//----------------------------.
void ThreadManager::CreateWorkers()
{
	// コア数に応じて作成する( 最低2本, メインスレッド分を1本残す ).
	const unsigned int HardwareNum	= std::thread::hardware_concurrency();
	const unsigned int WorkerNum	= ( HardwareNum <= 3 ) ? 2 : HardwareNum - 1;
	for ( unsigned int i = 0; i < WorkerNum; ++i ) {
		m_Workers.emplace_back( &ThreadManager::WorkerLoop, this );
	}
}

//----------------------------.
// ワーカースレッドの処理.
//----------------------------.
void ThreadManager::WorkerLoop()
{
	while ( true ) {
		PoolJob Job;
		{
			std::unique_lock<std::mutex> Lock( m_QueueMutex );
			// ジョブが来るか終了要求が来るまで待機する( 待機中は CPU を消費しない ).
			m_QueueCv.wait( Lock, [this]() { return m_IsShutdown || m_JobQueue.empty() == false; } );
			// 終了要求時も受付済みのジョブは全て実行してから抜ける.
			if ( m_IsShutdown && m_JobQueue.empty() ) return;
			Job = std::move( m_JobQueue.front() );
			m_JobQueue.pop();
		}
		// 停止要求済みのジョブは実行しない.
		if ( Job.pState->IsStopRequested == false ) {
			try {
				Job.Func();
			}
			catch ( const std::exception& e ) {
				Log::PushLogError( std::string( "ジョブで例外が発生 : " ) + e.what() );
			}
			catch ( ... ) {
				Log::PushLogError( "ジョブで不明な例外が発生" );
			}
		}
		FinishJob( Job.pState );
	}
}

//----------------------------.
// ジョブをプールキューへ積む.
//----------------------------.
bool ThreadManager::PushPoolJob( PoolJob&& Job, const bool IsForce )
{
	bool IsCreateWorker = false;
	{
		std::lock_guard<std::mutex> Lock( m_QueueMutex );
		// 終了中は受け付けない( 直列ジョブの続きは実行を保証するため IsForce で積む ).
		if ( m_IsShutdown && IsForce == false ) return false;
		// 初回使用時にワーカースレッドを作成する( 終了中は作成しない ).
		if ( m_Workers.empty() ) {
			if ( m_IsShutdown ) return false;
			CreateWorkers();
			IsCreateWorker = true;
		}
		m_JobQueue.push( std::move( Job ) );
	}
	// 待機中のワーカーを起こす.
	m_QueueCv.notify_one();

	// ログ出力はスレッドマネージャーを再帰的に呼ぶため、ロックの外で行う.
	if ( IsCreateWorker ) {
		Log::PushLogInfo( "ワーカースレッドを " + std::to_string( GetWorkerNum() ) + " 本作成" );
	}
	return true;
}

//----------------------------.
// ジョブの終了処理.
//----------------------------.
void ThreadManager::FinishJob( const JobHandle& pState )
{
	// 終了を通知する( Wait している側を起こす ).
	{
		std::lock_guard<std::mutex> Lock( pState->Mutex );
		pState->IsFinished = true;
	}
	pState->FinishCv.notify_all();

	// 完了コールバックをメインスレッド実行用に積む( Update 内で実行される ).
	if ( pState->OnFinish != nullptr ) {
		std::lock_guard<std::mutex> Lock( m_FinishMutex );
		m_FinishFuncs.emplace_back( pState->OnFinish );
	}

	// 直列ジョブの場合は同タグの次のジョブを実行キューへ積む.
	if ( pState->IsSequential ) OnSequentialJobFinished( pState->Tag );

	// 未完了数を減らす( 0になったら WaitAll を起こす. 専用スレッドは数えていない ).
	if ( pState->IsDedicated == false ) AddActiveJobNum( -1 );
}

//----------------------------.
// 直列ジョブ終了時の処理.
//----------------------------.
void ThreadManager::OnSequentialJobFinished( const std::string& Tag )
{
	PoolJob Next;
	{
		std::lock_guard<std::mutex> Lock( m_ChannelMutex );
		auto Itr = m_Channels.find( Tag );
		if ( Itr == m_Channels.end() ) return;

		SequentialChannel& Channel = Itr->second;
		// 待ちが無ければチャンネルを削除する.
		if ( Channel.Pending.empty() ) {
			m_Channels.erase( Itr );
			return;
		}
		Next = std::move( Channel.Pending.front() );
		Channel.Pending.pop_front();
	}

	// 次の同タグのジョブを実行キューへ積む( 終了中でも受付済みの分は実行しきる ).
	JobHandle pNextState = Next.pState;
	if ( PushPoolJob( std::move( Next ), true ) == false ) {
		// 積めなかった場合は終了扱いにする.
		FinishJob( pNextState );
	}
}

//----------------------------.
// 未完了ジョブ数の加算.
//----------------------------.
void ThreadManager::AddActiveJobNum( const int Add )
{
	bool IsAllFinished = false;
	{
		std::lock_guard<std::mutex> Lock( m_ActiveMutex );
		m_ActiveJobNum += Add;
		IsAllFinished = ( m_ActiveJobNum <= 0 );
	}
	// 全ジョブが終了した場合は WaitAll している側を起こす.
	if ( IsAllFinished ) m_ActiveCv.notify_all();
}

//----------------------------.
// 全スレッドの停止.
//----------------------------.
void ThreadManager::Shutdown()
{
	// 新規の受付を停止する( 停止済みの場合は何もしない ).
	{
		std::scoped_lock Lock( m_QueueMutex, m_DedicatedMutex );
		if ( m_IsShutdown ) return;
		m_IsShutdown = true;
	}

	// 専用スレッドへ停止要求を送り、終了を待つ.
	{
		std::lock_guard<std::mutex> Lock( m_DedicatedMutex );
		for ( auto& Dedicated : m_DedicatedThreads ) {
			Dedicated.pState->IsStopRequested = true;
		}
		for ( auto& Dedicated : m_DedicatedThreads ) {
			if ( Dedicated.Thread.joinable() ) Dedicated.Thread.join();
		}
		m_DedicatedThreads.clear();
	}

	// ワーカースレッドの終了を待つ( 受付済みのジョブは全て実行されてから止まる ).
	m_QueueCv.notify_all();
	for ( auto& Worker : m_Workers ) {
		if ( Worker.joinable() ) Worker.join();
	}
	m_Workers.clear();

	// 念のため残りを終了扱いにする( 通常はここまでに全て実行済み ).
	//	( FinishJob 内でチャンネルを再ロックしないよう、先に取り出してからクリアする ).
	std::vector<JobHandle> PendingStates;
	{
		std::lock_guard<std::mutex> Lock( m_ChannelMutex );
		for ( auto& Pair : m_Channels ) {
			for ( auto& Job : Pair.second.Pending ) PendingStates.emplace_back( Job.pState );
		}
		m_Channels.clear();
	}
	for ( auto& pState : PendingStates ) FinishJob( pState );
	{
		std::lock_guard<std::mutex> Lock( m_QueueMutex );
		while ( m_JobQueue.empty() == false ) {
			FinishJob( m_JobQueue.front().pState );
			m_JobQueue.pop();
		}
	}

	// 残った完了コールバックは実行せず破棄する( 呼び出し先が破棄されている恐れがあるため ).
	{
		std::lock_guard<std::mutex> Lock( m_FinishMutex );
		m_FinishFuncs.clear();
	}
}

#endif	// #ifdef ENABLE_THREAD.
