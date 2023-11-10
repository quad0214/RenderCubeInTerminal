#pragma once

#define USE_TSC
#define TSC_MEASURE_FREQ_SEC (0.5f)
#include <Windows.h>
#include <cstdint>
#include <intrin.h>
#include <strsafe.h>

class TimeStamper {
private:	
	enum class StamperStatus {
		BeforeInit,
		BeforeStart,
		During,		
	};

	static const int DEBUG_BUFFER_LEN = 1024;
	static const int STAMP_BUFFER_LEN = 256;

	// method
public:
	TimeStamper();
	virtual ~TimeStamper();

	bool Init();
	void Terminate();

	void Start(const wchar_t* stampName);
	void Stop();

private:
	inline uint64_t GetTicks();
	inline double CalculateSec(uint64_t ticks) const;


	// variable
private:	
	StamperStatus mStatus = StamperStatus::BeforeInit;
	wchar_t* mDebugBuffer = nullptr;	
	wchar_t* mCurStampName = nullptr;

	unsigned int mTscAux = 0;

	// freq
	uint64_t mQpcFreq = 0;
	uint64_t mTscFreq = 0;	

	// start ticks
	uint64_t mStartTicks = 0;
};