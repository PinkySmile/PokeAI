//
// Created by PinkySmile on 07/04/2020.
//

#ifdef __GNUG__
#include <cxxabi.h>
#endif
#include <regex>
#include <codecvt>
#include <SFML/Graphics.hpp>
#include <numeric>
#include "Utils.hpp"

namespace Utils
{
	std::wstring utf8ToWstring(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
	}

	std::string wstringToUtf8(const std::wstring& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.to_bytes(str);
	}

	std::string pathToString(const std::filesystem::path &path) {
#ifdef _GLIBCXX_FILESYSTEM_IS_WINDOWS
		std::string result;
		std::wstring p = path;

		result.reserve(p.size());
		for (wchar_t c : p)
			if (c > 255)
				result.push_back('?');
			else
				result.push_back(c);
		return result;
#else
		return path;
#endif
	}

	std::string getLastExceptionName()
	{
#ifdef __GNUG__
		int status;
		char *value;
		std::string name;

		auto val = abi::__cxa_current_exception_type();

		if (!val)
			return "No exception";

		value = abi::__cxa_demangle(val->name(), nullptr, nullptr, &status);
		name = value;
		free(value);
		return name;
#else
		return "Unknown exception";
#endif
	}

	std::string cleanPath(const std::string &path)
	{
		std::vector<std::string> files = {};
		std::istringstream iss{path};
		std::string item;

		while (std::getline(iss, item, static_cast<char>(std::filesystem::path::preferred_separator)))
			if (item == "..") {
				if (files.size() > 1)
					files.pop_back();
			} else if (item != "." && !item.empty())
				files.push_back(item);

		return std::accumulate(
			files.begin() + 1,
			files.end(),
#ifdef _WIN32
			files[0],
#else
			static_cast<char>(std::filesystem::path::preferred_separator) + files[0],
#endif
			[](const std::string &a, const std::string &b){
				return a + static_cast<char>(std::filesystem::path::preferred_separator) + b;
			}
		);
	}

	std::string toUpper(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::toupper(c); });
		return str;
	}

	std::string toLower(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
		return str;
	}
}
