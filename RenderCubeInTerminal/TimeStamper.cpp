#include "TimeStamper.h"
#include <iostream>
#include <cassert>

const float TimeStamper::TSC_FREQ_MEASURE_SEC = 0.5f;

TimeStamper::TimeStamper()
{
}

TimeStamper::~TimeStamper()
{
}

bool TimeStamper::Init()
{
	assert(TSC_FREQ_MEASURE_SEC != 0.0f);

	if (mState != StamperState::BeforeInit) {
		return false;
	}		

	std::thread initThread(&TimeStamper::InitWorker, this, TSC_FREQ_MEASURE_SEC);	
	mState = StamperState::DuringInit;
	initThread.detach();

	return true;
}

void TimeStamper::Terminate()
{
	// terminate init thread if it is working
	if (mState == StamperState::DuringInit) {
		mIsTerminateInitThread = true;
		while (mState == StamperState::DuringInit) {}
		mIsTerminateInitThread = false;
	}

	if (mDebugBuffer) {
		delete[] mDebugBuffer;
		mDebugBuffer = nullptr;
	}

	if (mCurStampName) {
		delete[] mCurStampName;
		mCurStampName = nullptr;
	}		

	mState = StamperState::BeforeInit;
}

void TimeStamper::Start(const wchar_t* stampName)
{
	if (mState != StamperState::BeforeStart) {
		__debugbreak();
	}
	mState = StamperState::During;
	
	StringCchCopyW(mCurStampName, STAMP_BUFFER_LEN, stampName);		

	mStartTicks = GetTicks();
}

void TimeStamper::Stop()
{	
	if (mState != StamperState::During) {
		__debugbreak();
	}	

	uint64_t intervalTicks = GetTicks() - mStartTicks;
	double intervalSec = CalculateSec(intervalTicks);
	
	const wchar_t* format = L"[Time Stamp] %s : %fs\n";
	StringCbPrintfW(mDebugBuffer, DEBUG_BUFFER_LEN * sizeof(wchar_t), format, mCurStampName, intervalSec);
	OutputDebugStringW(mDebugBuffer);

	mState = StamperState::BeforeStart;
}

inline double TimeStamper::CalculateSec(uint64_t ticks) const
{
#ifdef USE_TSC
	return (double)ticks / mTscFreq;
#else
	return (double)ticks / mQpcFreq;
#endif
}

inline uint64_t TimeStamper::GetTicks()
{
#ifdef USE_TSC	
	return __rdtscp(&mTscAux);
#else
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);

	return static_cast<uint64_t>(*reinterpret_cast<int64_t*>(&ticks));
#endif
}

void TimeStamper::InitWorker(float tscFreqMeasureSec)
{
	// allocate string buffers
	mDebugBuffer = new wchar_t[DEBUG_BUFFER_LEN];
	ZeroMemory(mDebugBuffer, sizeof(wchar_t) * DEBUG_BUFFER_LEN);

	mCurStampName = new wchar_t[STAMP_BUFFER_LEN];
	ZeroMemory(mDebugBuffer, sizeof(wchar_t) * STAMP_BUFFER_LEN);

	// set freq
	LARGE_INTEGER qpcFreq;
	QueryPerformanceFrequency(&qpcFreq);
	mQpcFreq = qpcFreq.QuadPart;

#ifdef USE_TSC	
	// todo check isable to use tsc

	// measure tsc
	uint64_t tscFreq;

	// todo check isable to use tsc
	int64_t measureQpc = tscFreqMeasureSec * qpcFreq.QuadPart;

	LARGE_INTEGER startQpc;
	LARGE_INTEGER curQpc;
	uint64_t startTsc;
	uint64_t curTsc;
	unsigned int tscAux;
	QueryPerformanceCounter(&startQpc);
	startTsc = __rdtscp(&mTscAux);
	QueryPerformanceCounter(&curQpc);
	curTsc = __rdtscp(&tscAux);
	while (curQpc.QuadPart - startQpc.QuadPart < measureQpc && !mIsTerminateInitThread) {
		QueryPerformanceCounter(&curQpc);
		curTsc = __rdtscp(&tscAux);
	}

	mTscFreq = static_cast<uint64_t>((curTsc - startTsc) * (1.0 / tscFreqMeasureSec));
#endif
	
	mState = StamperState::BeforeStart;
}

bool TimeStamper::IsInit() const
{
	return mState == StamperState::BeforeStart;
}
