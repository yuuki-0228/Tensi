#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_EASING
#include "..\..\Global.h"

/************************************************
*	イージングクラス.
**/
namespace Easing {
	// イージング関数.
	//	 各イージングの動作は以下で確認することができます.
	//	 https://easings.net/ja.
	float Linear(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );

	float InSine(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float OutSine(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float InOutSine(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );

	float InCubic(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float OutCubic(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float InOutCubic(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );

	float InQuint(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float OutQuint(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float InOutQuint(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );

	float InCirc(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float OutCirc(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float InOutCirc(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );

	float InQuad(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float OutQuad(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float InOutQuad(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );

	float InQuart(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float OutQuart(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float InOutQuart(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	
	float InExpo(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float OutExpo(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float InOutExpo(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );

	float InBack(		float Time, float TotalTime, float Back, float Max = 1.0f, float Min = 0.0f );
	float OutBack(		float Time, float TotalTime, float Back, float Max = 1.0f, float Min = 0.0f );
	float InOutBack(	float Time, float TotalTime, float Back, float Max = 1.0f, float Min = 0.0f );
	
	float InBounce(		float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float OutBounce(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );
	float InOutBounce(	float Time, float TotalTime, float Max = 1.0f, float Min = 0.0f );

	// イージングリストの取得.
	std::vector<std::string> GetEasingList();

	// イージングリストからイージングを行う.
	float Easing( const std::string& Name, float Time, float TotalTime, float Back, float Max = 1.0f, float Min = 0.0f );
};
#endif