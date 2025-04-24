#pragma once

#include <string>

uint32_t binary_stoui32(const std::string& _String)
{
	uint32_t result = 0;

	size_t size = _String.size() - 1;
	for(int64_t i = (int64_t)size; i >= 0; i--)
	{
		if(_String[i] == '1') result += (uint32_t)pow(2, _String.size() - 1 - i);
	}

	return result;
}

uint16_t binary_stoui16(const std::string& _String)
{
	uint16_t result = 0;
	size_t size = _String.size() - 1;
	for(int64_t i = (int64_t)size; i >= 0; i--)
	{
		if(_String[i] == '1') result += (uint16_t)pow(2, _String.size() - 1 - i);
	}

	return result;
}

std::wstring stows(const std::string& _String)
{
	std::wstring ws;
	ws.resize(_String.size()+1);

	size_t b;
	mbstowcs_s(&b, &ws[0], ws.size(), _String.c_str(), _String.size());
	return ws;
}

std::string wstos(const std::wstring& _WString)
{
	std::string s;
	s.resize(_WString.size()+1);

	size_t b;
	wcstombs_s(&b, &s[0], s.size(), _WString.c_str(), _WString.size());
	return s;
}