#pragma once

//---------------------------.
//	探索の保存データ.
//	アプリを終了して再度立ち上げても復帰できる情報.
//---------------------------.
struct stExploreSaveData {
	bool		IsExploring;		// 探索中か.
	long long	StartTime;			// 探索の開始時刻( time_t ).
	long long	EndTime;			// 探索の完了予定時刻( time_t ).
	char		FolderPath[260];	// 探索中のフォルダのフルパス.
} typedef SExploreSaveData;
