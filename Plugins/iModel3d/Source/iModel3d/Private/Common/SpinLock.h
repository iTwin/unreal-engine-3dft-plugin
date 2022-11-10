#pragma once

#include "CoreMinimal.h"

struct FSpinLock
{
	void Lock()
	{
		//QUICK_SCOPE_CYCLE_COUNTER(STAT_FMeshComponentManager_Lock);
		while (Flag.test_and_set(std::memory_order_acquire)) {}
	}
	void Unlock() { Flag.clear(std::memory_order_release); }
private:
	std::atomic_flag Flag = ATOMIC_FLAG_INIT;
};
