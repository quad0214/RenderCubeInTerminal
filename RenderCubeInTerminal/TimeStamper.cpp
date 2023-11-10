#include "TimeStamper.h"


TimeStamper::TimeStamper()
{
}

TimeStamper::~TimeStamper()
{
}

bool TimeStamper::Init()
{
	if (mStatus != StamperStatus::BeforeInit) {
		return false;
	}
	mStatus = StamperStatus::BeforeStart;

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
	// todo check is valid tsc
	int64_t measureQpc = TSC_MEASURE_FREQ_SEC * mQpcFreq;

	LARGE_INTEGER startQpc;
	LARGE_INTEGER curQpc;
	uint64_t startTsc;
	uint64_t curTsc;
	QueryPerformanceCounter(&startQpc);
	startTsc = __rdtscp(&mTscAux);
	QueryPerformanceCounter(&curQpc);	
	curTsc = __rdtscp(&mTscAux);
	while (curQpc.QuadPart - startQpc.QuadPart < measureQpc) {
		QueryPerformanceCounter(&curQpc);
		curTsc = __rdtscp(&mTscAux);
	}

	mTscFreq = static_cast<uint64_t>((curTsc - startTsc) * (1.0 / TSC_MEASURE_FREQ_SEC));	
#endif

	return true;
}

void TimeStamper::Terminate()
{
	if (mDebugBuffer) {
		delete[] mDebugBuffer;
		mDebugBuffer = nullptr;
	}

	if (mCurStampName) {
		delete[] mCurStampName;
		mCurStampName = nullptr;
	}	

	mStatus = StamperStatus::BeforeInit;
}

void TimeStamper::Start(const wchar_t* stampName)
{
	if (mStatus != StamperStatus::BeforeStart) {
		__debugbreak();
	}
	mStatus = StamperStatus::During;
	
	StringCchCopyW(mCurStampName, STAMP_BUFFER_LEN, stampName);		

	mStartTicks = GetTicks();
}

void TimeStamper::Stop()
{	
	if (mStatus != StamperStatus::During) {
		__debugbreak();
	}	

	uint64_t intervalTicks = GetTicks() - mStartTicks;
	double intervalSec = CalculateSec(intervalTicks);
	
	const wchar_t* format = L"[Time Stamp] %s : %fs\n";
	StringCbPrintfW(mDebugBuffer, DEBUG_BUFFER_LEN * sizeof(wchar_t), format, mCurStampName, intervalSec);
	OutputDebugStringW(mDebugBuffer);

	mStatus = StamperStatus::BeforeStart;
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
