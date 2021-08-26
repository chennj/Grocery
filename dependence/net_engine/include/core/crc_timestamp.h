#ifndef _CRC_TIMESTAMP_H_
#define _CRC_TIMESTAMP_H_
// file: crc_timestamp.h

#include <chrono>

using namespace std::chrono;

class CRCTime
{
public:
	// 获取当前时间戳 (毫秒)
	inline static time_t getNowInMilliSec()
	{
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};

class CRCTimestamp
{
public:
	CRCTimestamp()
	{
		update();
	}
	~CRCTimestamp()
	{

	}

public:
	/**
	*	fresh current time
	*/
	inline void update()
	{
		_begin = high_resolution_clock::now();
	}

	/**
	*	get current time(second)
	*/
	inline double getElapsedSecond()
	{
		return getElapsedTimeInMicroSec() * 0.000001;
	}

	/**
	*	get current time(millisec)
	*/
	inline double getElapsedTimeInMilliSec()
	{
		return this->getElapsedTimeInMicroSec() * 0.001;
	}

	/**
	*	get current time(microsec)
	*/
	inline long long getElapsedTimeInMicroSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}

protected:
	time_point<high_resolution_clock> _begin;
};

#endif