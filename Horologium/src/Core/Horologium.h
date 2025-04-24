#pragma once

#ifdef HOROLOGIUM_EXPORTS
	#define HOROLOGIUM_API __declspec(dllexport)
#else
	#define HOROLOGIUM_API __declspec(dllimport)
#endif // HOROLOGIUM_EXPORTS

namespace Horologium {

	extern "C" class Instrumentor;
	extern "C" class InstrumentorData;
	extern "C" class Timer;
	extern "C" class Serializer;
	extern "C" class Deserializer;

}