#include "pch.h"

#include "Timer.h"

namespace Horologium {

	// Constructor
	//
	// Creates a timer object with a name and sets it's start time, elapsed time, and frequency to 0, and the bIsRunning state flag to false
	//
	// @param const std::wstring& - Name for the Timer (can be changed later)
	Timer::Timer(const std::wstring& _szName):
		m_szName(_szName),
		m_uiStartTime(0),
		m_uiElapsedTime(0),
		m_uiFrequency(0),
		m_bIsRunning(false)
	{
	}

	// start(void)
	//
	// Sets the start time to the current time since epoch in milliseconds, increments frequency, and sets the bIsRunning state flag to true
	//
	// @return const uint64_t - the recorded start time
	// @exception runtime_error - Timer::start() - Timer is already running
	const uint64_t Timer::start(void)
	{
		if(m_bIsRunning)
		{
			throw std::runtime_error("Timer::start() - Timer is already running");
			return 0;
		}

		m_uiStartTime = std::chrono::steady_clock::now().time_since_epoch().count() / 1000000;	// milliseconds
		m_uiFrequency++;
		m_bIsRunning = true;

		return m_uiStartTime;
	}

	// stop(void)
	//
	// Sets the elapsed time to the current time since epoch in milliseconds - the start time, and sets the bIsRunning state flag to false
	//
	// @return const uint64_t - the last recorded elapsed time
	// @exception runtime_error - Timer::stop() - Timer is not running
	const uint64_t Timer::stop(void)
	{
		if(!m_bIsRunning)
		{
			throw std::runtime_error("Timer::stop() - Timer is not running");
			return 0;
		}

		uint64_t uiEndTime = std::chrono::steady_clock::now().time_since_epoch().count() / 1000000;	// milliseconds
		m_bIsRunning = false;

		m_uiElapsedTime = uiEndTime - m_uiStartTime;

		return m_uiElapsedTime;
	}

	// reset(void)
	//
	// Sets start time, elapsed time, and frequency to 0, and sets the bIsRunning state flag to false
	//
	// Note: reset() will stop the timer but erase all progress. Be careful not to place a reset() call between start() and stop() calls.
	// Example Usage:
	//		start(); stop(); reset();	<- Ok; Resets frequency
	//		start(); reset(); start();	<- Ok; Resets frequency and restarts the timer
	//		start(); reset(); stop();	<- NOT Ok; stop() will throw an exception as timer is not running
	void Timer::reset(void)
	{
		m_uiStartTime = 0;
		m_uiElapsedTime = 0;
		m_uiFrequency = 0;
		m_bIsRunning = false;
	}
}