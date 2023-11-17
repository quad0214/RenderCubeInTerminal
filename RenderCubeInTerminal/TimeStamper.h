#pragma once

#define USE_TSC

#include <Windows.h>
#include <cstdint>
#include <intrin.h>
#include <strsafe.h>
#include <atomic>
#include <thread>

/// <summary>
/// Debug interval time of specific code section that you mark.
/// thread unsafe. Must use on main thread.
/// </summary>
class TimeStamper {
private:	
	enum class StamperState {
		BeforeInit,
		DuringInit,
		BeforeStart,
		During,		
	};

	static const int DEBUG_BUFFER_LEN = 1024;
	static const int STAMP_BUFFER_LEN = 256;
	static const float TSC_FREQ_MEASURE_SEC;

	// method
public:
	TimeStamper();
	virtual ~TimeStamper();

	bool Init();
	void Terminate();

	void Start(const wchar_t* stampName);
	void Stop();

	bool IsInit() const;

private:
	void InitWorker(float tscFreqMeasureSec);

	inline uint64_t GetTicks();
	inline double CalculateSec(uint64_t ticks) const;


	// variable
private:	
	std::atomic<StamperState> mState{ StamperState::BeforeInit };
	wchar_t* mDebugBuffer = nullptr;	
	wchar_t* mCurStampName = nullptr;
	
	std::atomic<bool> mIsTerminateInitThread = false;
	
	// freq
	uint64_t mQpcFreq = 0;
	uint64_t mTscFreq = 0;	
	unsigned int mTscAux = 0;

	// start ticks
	uint64_t mStartTicks = 0;
};