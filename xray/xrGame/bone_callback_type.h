#pragma once

enum EBoneCallbackType{
	bctDummy			= u32(0),	// 0 - required!!!
	bctPhysics,
	bctCustom,
	bctHour,
	bctMinute,
	bctForceU32			= u32(-1),
};
