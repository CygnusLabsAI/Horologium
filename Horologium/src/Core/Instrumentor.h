#pragma once

#include <filesystem>
#include "Horologium.h"
#include "Timer.h"

namespace Horologium {

	HOROLOGIUM_API class InstrumentorData
	{
		public:
			InstrumentorData(const std::thread::id& _ThreadId, const std::wstring& _szName, size_t _uiStackLevel, uint64_t _uiStartTime = 0, uint64_t _uiElapsedTime = 0);
			InstrumentorData(size_t _uiThreadId, const std::wstring& _szName, size_t _uiStackLevel, uint64_t _uiStartTime = 0, uint64_t _uiElapsedTime = 0);

			// Getters & Setters
			inline const size_t& getThreadId(void) const { return m_ThreadId; }
			inline const std::wstring& getName(void) const { return m_szName; }
			inline const size_t& getStackLevel(void) const { return m_uiStackLevel; }
			inline const uint64_t& getStartTime(void) const { return m_uiStartTime; }
			inline const uint64_t& getElapsedTime(void) const { return m_uiElapsedTime; }

			inline void setStartTime(uint64_t _uiStartTime) { m_uiStartTime = _uiStartTime; }
			inline void setElapsedTime(uint64_t _uiElapsedTime) { m_uiElapsedTime = _uiElapsedTime; }

		private:
			size_t m_ThreadId;
			std::wstring m_szName;
			size_t m_uiStackLevel;
			uint64_t m_uiStartTime;
			uint64_t m_uiElapsedTime;
	};

	HOROLOGIUM_API class Instrumentor
	{
		public:
			HOROLOGIUM_API Instrumentor(bool _bDebug = false);
			HOROLOGIUM_API ~Instrumentor(void);

			HOROLOGIUM_API void BeginProfile(const std::thread::id& _ThreadId, const std::wstring& _szName);
			HOROLOGIUM_API bool EndProfile(const std::thread::id& _ThreadId, const std::wstring& _szName);

			HOROLOGIUM_API void saveData(const std::wstring& _szFileName);
			HOROLOGIUM_API void loadData(const std::wstring& _szFileName);


		private:
			std::unordered_map<std::thread::id, std::stack<Timer>> m_ProfileStacks;
			std::unordered_map<std::thread::id, std::stack<size_t>> m_DataIndexStacks;
			bool m_bDebug;
			std::atomic<unsigned int> m_ProfileCount;
			std::vector<InstrumentorData> m_InstrumentorData;

	};

	HOROLOGIUM_API class Serializer
	{
		public:
			Serializer(std::vector<InstrumentorData>* _pInstrumentorData, std::filesystem::path _OutputFilePath);

			bool serialize(bool _bDebug = false);

		private:
			enum class DataType
			{
				THREAD_ID,
				FUNCTION_NAME,
				STACK_LEVEL,
				ELAPSED_TIME,
				LENGTH_OF_FUNCTION_NAME
			};

			struct Node
			{
				std::string szValue;
				size_t uiFrequency = 0;
				Node* pLeft = nullptr;
				Node* pRight = nullptr;
			};

			bool outputBinary(void);
			bool outputText(void);
			bool compress_Huffman(const std::filesystem::path& _FilePath);
			void generateHuffmanCodes(std::map<std::string, std::string>& _dataCodeMap, Node* _pNode, std::string code = "");
			void normalizeHuffmanCodes(std::map<std::string, std::string>& _dataCodeMap);

			bool compress_LZ77(const std::filesystem::path& _FilePath);

			std::vector<InstrumentorData>* m_pInstrumentorData;
			std::filesystem::path m_OutputFilePath;
	};

	HOROLOGIUM_API class Deserializer
	{
		public:
			Deserializer(std::vector<InstrumentorData>* _pInstrumentorData, std::filesystem::path _InputFilePath);

			bool deserialize(void);

		private:
			std::vector<InstrumentorData>* m_pInstrumentorData;
			std::filesystem::path m_InputFilePath;
	};
}
