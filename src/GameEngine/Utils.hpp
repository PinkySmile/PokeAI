//
// Created by Gegel85 on 07/04/2020.
//

#ifndef GAMEENGINE_UTILS_HPP
#define GAMEENGINE_UTILS_HPP


#include <string>
#include <vector>
#include <filesystem>

#ifndef _WIN32
#define MB_ICONERROR 1
#else
#include <windows.h>
#endif


namespace Utils
{
	std::string toUpper(std::string str);
	std::string toLower(std::string str);

	//! @brief Get the last Exception Name
	//! @details Return the last type of Exception name
	//! @return std::string The last Exception name
	std::string getLastExceptionName();

	//! @brief Convert a wstring into an UTF-8 string
	//! @param str String to convert in UTF-8
	//! @return std::string Converted String
	std::string wstringToUtf8(const std::wstring &str);

	//! @brief Convert a UTF-8 into a wstring
	//! @param str String to convert in wstring
	//! @return std::wstring Converted String
	std::wstring utf8ToWstring(const std::string &str);

	//! @brief Convert a std::filesystem::path to string UTF-8
	//! @param path filesystem path to convert
	//! @return std::string Converted Path in String
	std::string pathToString(const std::filesystem::path &path);

	//! @brief clean the . and .. in relative paths
	//! @param path filesystem path to clean
	//! @return std::string Converted Path in String
	std::string cleanPath(const std::string &path);
}


#endif //GAMEENGINE_UTILS_HPP
