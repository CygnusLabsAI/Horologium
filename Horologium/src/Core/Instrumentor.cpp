#include "pch.h"

#include "Instrumentor.h"
#include "..\Utils\std_string_conversion.h"

namespace Horologium {

	InstrumentorData::InstrumentorData(const std::thread::id& _ThreadId, const std::wstring& _szName, size_t _uiStackLevel, uint64_t _uiStartTime, uint64_t _uiElapsedTime):
		InstrumentorData(static_cast<size_t>(std::hash<std::thread::id>{}(_ThreadId)), _szName, _uiStackLevel, _uiStartTime, _uiElapsedTime)
	{
	}

	InstrumentorData::InstrumentorData(size_t _uiThreadId, const std::wstring& _szName, size_t _uiStackLevel, uint64_t _uiStartTime, uint64_t _uiElapsedTime):
		m_ThreadId(_uiThreadId),
		m_szName(_szName),
		m_uiStackLevel(_uiStackLevel),
		m_uiStartTime(_uiStartTime),
		m_uiElapsedTime(_uiElapsedTime)
	{
	}

	Instrumentor::Instrumentor(bool _bDebug):
		m_bDebug(_bDebug),
		m_ProfileCount(0)
	{
	}

	Instrumentor::~Instrumentor(void)
	{
	}

	void Instrumentor::BeginProfile(const std::thread::id& _ThreadId, const std::wstring& _szName)
	{
		auto& timerStack = m_ProfileStacks[_ThreadId];
		auto& indexStack = m_DataIndexStacks[_ThreadId];
		timerStack.push(Timer(_szName));
		InstrumentorData data(_ThreadId, _szName, timerStack.size() - 1, timerStack.top().start());
		m_InstrumentorData.push_back(data);
		indexStack.push(m_InstrumentorData.size() - 1);
		++m_ProfileCount;
	}

	bool Instrumentor::EndProfile(const std::thread::id& _ThreadId, const std::wstring& _szName)
	{
		auto itTimer = m_ProfileStacks.find(_ThreadId);
		auto itIndex = m_DataIndexStacks.find(_ThreadId);
		if(itTimer != m_ProfileStacks.end() && itIndex != m_DataIndexStacks.end())
		{
			m_InstrumentorData[itIndex->second.top()].setElapsedTime(itTimer->second.top().stop());
			itTimer->second.pop();
			itIndex->second.pop();
			--m_ProfileCount;
			return true;
		}
		return false;
	}

	HOROLOGIUM_API void Instrumentor::saveData(const std::wstring& _szFileName)
	{
		std::filesystem::path filePath(_szFileName);
		Serializer serializer(&m_InstrumentorData, filePath);
		serializer.serialize();
	}

	HOROLOGIUM_API void Instrumentor::loadData(const std::wstring& _szFileName)
	{
		std::filesystem::path filePath(_szFileName);
		Deserializer deserializer(&m_InstrumentorData, filePath);
		deserializer.deserialize();
	}
	
	Serializer::Serializer(std::vector<InstrumentorData>* _pInstrumentorData, std::filesystem::path _OutputFilePath):
		m_pInstrumentorData(_pInstrumentorData),
		m_OutputFilePath(_OutputFilePath)
	{
	}

	bool Serializer::serialize(bool _bDebug)
	{
		bool result = outputBinary();
		if(_bDebug) result = result && outputText();
		return result;
	}

	bool Serializer::outputBinary(void)
	{
		std::string outputFile(m_OutputFilePath.string() + ".hid");
		FILE* out;
		fopen_s(&out, outputFile.c_str(), "wb");
		if(out)
		{
			for(size_t i = 0; i < m_pInstrumentorData->size(); i++)
			{
				// Thread ID
				fwrite(&m_pInstrumentorData->at(i).getThreadId(), sizeof(size_t), 1, out);

				// Profiled Function Name
				size_t fnNameLen = m_pInstrumentorData->at(i).getName().size();
				fwrite(&fnNameLen, sizeof(size_t), 1, out);
				if(fnNameLen > 255)
				{
					fclose(out);
					return false;
				}
				std::string fnNameStr;
				fnNameStr.resize(fnNameLen);
				size_t b;
				wcstombs_s(&b, &fnNameStr[0], fnNameLen + 1, m_pInstrumentorData->at(i).getName().c_str(), fnNameLen);
				fwrite(fnNameStr.c_str(), sizeof(char), fnNameLen, out);

				// Stack Level
				fwrite(&m_pInstrumentorData->at(i).getStackLevel(), sizeof(size_t), 1, out);

				// Start Time
				fwrite(&m_pInstrumentorData->at(i).getStartTime(), sizeof(uint64_t), 1, out);

				// Elapsed Time
				fwrite(&m_pInstrumentorData->at(i).getElapsedTime(), sizeof(uint64_t), 1, out);
			}

			fclose(out);

			// Compress using LZ77
			std::filesystem::path file(outputFile);
			if(!compress_Huffman(file)) return false;

			return true;
		}
		return false;
	}

	bool Serializer::outputText(void)
	{
		


		return false;
	}

	bool Serializer::compress_Huffman(const std::filesystem::path& _FilePath)
	{
		std::vector<InstrumentorData> data;
		std::map<std::string, size_t> frequencyMap;

		if(std::filesystem::exists(_FilePath))
		{
			std::ifstream in(_FilePath.string(), std::ios::binary | std::ios::in);

			while(!in.eof())
			{
				size_t uiThreadID = 0;
				in.read(reinterpret_cast<char*>(&uiThreadID), sizeof(size_t));
				if(in.eof()) break;
				size_t uiFnNameLen = 0;
				in.read(reinterpret_cast<char*>(&uiFnNameLen), sizeof(size_t));
				//frequencyMap[std::to_string(uiFnNameLen)]++;
				std::string szFnNameStr;
				szFnNameStr.resize(uiFnNameLen);
				in.read(&szFnNameStr[0], uiFnNameLen);
				std::wstring szFnName;
				szFnName.resize(uiFnNameLen);
				size_t b;
				mbstowcs_s(&b, &szFnName[0], uiFnNameLen + 1, szFnNameStr.c_str(), uiFnNameLen);
				size_t uiStackLevel = 0;
				in.read(reinterpret_cast<char*>(&uiStackLevel), sizeof(size_t));
				uint64_t uiStartTime = 0;
				in.read(reinterpret_cast<char*>(&uiStartTime), sizeof(uint64_t));
				uint64_t uiElapsedTime = 0;
				in.read(reinterpret_cast<char*>(&uiElapsedTime), sizeof(uint64_t));

				InstrumentorData readData(uiThreadID, szFnName, uiStackLevel, uiStartTime, uiElapsedTime);
				data.push_back(readData);
			}
		}

		for(size_t i = 0; i < data.size(); i++)
		{
			frequencyMap[std::to_string(data[i].getThreadId())]++;
			frequencyMap[std::to_string(data[i].getName().size())]++;
			frequencyMap[wstos(data[i].getName())]++;
			frequencyMap[std::to_string(data[i].getStackLevel())]++;
			frequencyMap[std::to_string(data[i].getStartTime())]++;
			frequencyMap[std::to_string(data[i].getElapsedTime())]++;
		}

		std::map<std::string, std::string> dataCodeMap;

		auto compareNodes = [](const Node* _pLeft, const Node* _pRight)
		{
			return _pLeft->uiFrequency > _pRight->uiFrequency;
		};

		std::priority_queue < Node*, std::vector<Node*>, decltype(compareNodes)> priorityQueue(compareNodes);

		for(const auto& pair : frequencyMap)
		{
			Node* newNode = new Node();
			newNode->szValue = pair.first;
			newNode->uiFrequency = pair.second;
			priorityQueue.push(newNode);
		}

		Node* pRoot = nullptr;
		while(!priorityQueue.empty())
		{
			Node* pLeft = priorityQueue.top();
			priorityQueue.pop();
			if(priorityQueue.empty())
			{
				pRoot = pLeft;
				break;
			}

			Node* pRight = priorityQueue.top();
			priorityQueue.pop();

			Node* newNode = new Node();
			newNode->uiFrequency = pLeft->uiFrequency + pRight->uiFrequency;

			newNode->pLeft = pLeft;
			newNode->pRight = pRight;

			priorityQueue.push(newNode);
		}

		generateHuffmanCodes(dataCodeMap, pRoot);
		normalizeHuffmanCodes(dataCodeMap);

		std::vector<uint16_t> compressedData;

		for(size_t i = 0; i < data.size(); i++)
		{
			compressedData.push_back(binary_stoui32(dataCodeMap[std::to_string(data[i].getThreadId())]));
			compressedData.push_back(binary_stoui32(dataCodeMap[std::to_string(data[i].getName().size())]));
			compressedData.push_back(binary_stoui32(dataCodeMap[wstos(data[i].getName())]));
			compressedData.push_back(binary_stoui32(dataCodeMap[std::to_string(data[i].getStackLevel())]));
			compressedData.push_back(binary_stoui32(dataCodeMap[std::to_string(data[i].getStartTime())]));
			compressedData.push_back(binary_stoui32(dataCodeMap[std::to_string(data[i].getElapsedTime())]));
		}

		std::ofstream out(_FilePath.string(), std::ios::binary | std::ios::out);

		out << "HLGC";
		size_t dictionarySize = dataCodeMap.size();
		out.write(reinterpret_cast<const char*>(&dictionarySize), sizeof(size_t));
		for(auto& pair : dataCodeMap)
		{
			uint8_t symbolSize = (uint8_t)pair.first.size();
			out.write(reinterpret_cast<const char*>(&symbolSize), sizeof(uint8_t));
			const char* symbol = pair.first.c_str();
			out.write(&symbol[0], symbolSize);
			uint16_t code = binary_stoui16(pair.second);
			out.write(reinterpret_cast<const char*>(&code), sizeof(uint16_t));
		}
		for(size_t i = 0; i < compressedData.size(); i++)
		{
			uint16_t code = compressedData[i];
			out.write(reinterpret_cast<const char*>(&code), sizeof(uint16_t));
		}

		return true;
	}

	void Serializer::generateHuffmanCodes(std::map<std::string, std::string>& _dataCodeMap, Node* _pNode, std::string _szCode)
	{
		if(!_pNode) return;
		if(!_pNode->pLeft && !_pNode->pRight)
		{
			_dataCodeMap[_pNode->szValue] = _szCode;
		}

		generateHuffmanCodes(_dataCodeMap, _pNode->pLeft, _szCode + "0");
		generateHuffmanCodes(_dataCodeMap, _pNode->pRight, _szCode + "1");
	}

	void Serializer::normalizeHuffmanCodes(std::map<std::string, std::string>& _dataCodeMap)
	{
		
		for(auto& pair : _dataCodeMap)
		{
			std::string padding;
			size_t size = pair.second.size();
			if(16 - size > 0)
			{
				size_t paddingSize = 16 - pair.second.size();
				for(size_t i = 0; i < paddingSize; i++)
				{
					padding.append("0");
				}

				switch(pair.second.c_str()[0])
				{
					case '0':	// Left side of Tree, padding goes to the left
						padding[padding.size() - 1] = '1';
						pair.second = padding.append(pair.second);
						break;
					case '1':	// Right side of the Tree, padding goes to the right
						pair.second = pair.second.append(padding);
						break;
				}
			}
			else throw std::runtime_error("Serializer::normalizeHuffmanCodes - code exceeds padding requirements");
		}
	}

	bool Serializer::compress_LZ77(const std::filesystem::path& _FilePath)
	{
		std::vector<uint8_t> dictionary;
		std::vector<uint8_t> window;
		window.resize(2048);
		if(std::filesystem::exists(_FilePath))
		{
			std::ifstream in(_FilePath.string(), std::ios::binary | std::ios::in);
			in.read(reinterpret_cast<char*>(window.data()), window.size());
			if((size_t)in.gcount() < window.size()) window.resize((size_t)in.gcount());

		}
		return false;
	}

	Deserializer::Deserializer(std::vector<InstrumentorData>* _pInstrumentorData, std::filesystem::path _InputFilePath):
		m_pInstrumentorData(_pInstrumentorData),
		m_InputFilePath(_InputFilePath)
	{
	}

	bool Deserializer::deserialize(void)
	{
		if(std::filesystem::exists(m_InputFilePath) && m_pInstrumentorData != nullptr)
		{
			std::ifstream in(m_InputFilePath.string(), std::ios::binary | std::ios::in);

			// Check for validation string to ensure file is the correct format
			std::string validation;
			validation.resize(4);
			in.read(&validation[0], 4);
			if(validation != "HLGC") return false;

			// Get dictionary
			size_t dictionarySize = 0;
			in.read(reinterpret_cast<char*>(&dictionarySize), sizeof(size_t));

			std::map<uint16_t, std::string> dictionary;
			for(size_t i = 0; i < dictionarySize; i++)
			{
				// Symbol character size
				uint8_t symbolLen = 0;
				in.read(reinterpret_cast<char*>(&symbolLen), sizeof(uint8_t));

				// Symbol
				std::string symbol;
				symbol.resize(symbolLen);
				in.read(&symbol[0], symbolLen);

				// Code
				uint16_t code = 0;
				in.read(reinterpret_cast<char*>(&code), sizeof(uint16_t));

				dictionary[code] = symbol;
			}

			// Rebuild data from dictionary
			while(!in.eof())
			{
				uint32_t code = 0;

				// ThreadId
				size_t uiThreadId;
				in.read(reinterpret_cast<char*>(&code), sizeof(uint16_t));
				if(in.eof()) break;
				uiThreadId = strtoull(dictionary[code].c_str(), nullptr, 10);

				// Function Name Len
				uint8_t uiFnNameLn;
				in.read(reinterpret_cast<char*>(&code), sizeof(uint16_t));
				
				uiFnNameLn = (uint8_t)strtoull(dictionary[code].c_str(), nullptr, 10);

				// Function Name
				std::string szFnName;
				szFnName.resize(uiFnNameLn);

				in.read(reinterpret_cast<char*>(&code), sizeof(uint16_t));
				szFnName = dictionary[code];

				size_t uiStackLevel;
				in.read(reinterpret_cast<char*>(&code), sizeof(uint16_t));
				uiStackLevel = strtoull(dictionary[code].c_str(), nullptr, 10);

				uint64_t uiStartTime;
				in.read(reinterpret_cast<char*>(&code), sizeof(uint16_t));
				uiStartTime = strtoull(dictionary[code].c_str(), nullptr, 10);

				uint64_t uiElapsedTime;
				in.read(reinterpret_cast<char*>(&code), sizeof(uint16_t));
				uiElapsedTime = strtoull(dictionary[code].c_str(), nullptr, 10);

				InstrumentorData readData(uiThreadId, stows(szFnName), uiStackLevel, uiStartTime, uiElapsedTime);
				m_pInstrumentorData->push_back(readData);

			}
		}

		return true;
	}

}