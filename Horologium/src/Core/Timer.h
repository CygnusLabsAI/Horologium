#pragma once

#include "Horologium.h"

namespace Horologium {

	HOROLOGIUM_API class Timer
	{
		public:
			Timer(const std::wstring& _szName = L"MyTimer");

			const uint64_t start(void);
			const uint64_t stop(void);
			void reset(void);

			// Getters & Setters

			inline const std::wstring& getName(void) const { return m_szName; }
			inline const uint64_t getStartTime(void) const { return m_uiStartTime; }
			inline const uint64_t getElapsedTime(void) const { return m_uiElapsedTime; }
			inline const uint32_t getFrequency(void) const { return m_uiFrequency; }

			inline void setName(const std::wstring& _szName) { m_szName = _szName; }

		private:
			std::wstring m_szName;		// Name of the Timer (0-Terminated Wide-String)
			uint64_t m_uiStartTime;		// Start Time (in milliseconds)
			uint64_t m_uiElapsedTime;	// Elapsed Time (in milliseconds)
			uint32_t m_uiFrequency;		// Number of times Timer has been started
			bool m_bIsRunning;			// Flag for recording Elapsed Time
	};
}