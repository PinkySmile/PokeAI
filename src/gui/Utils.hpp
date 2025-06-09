//
// Created by Gegel85 on 07/04/2020.
//

#ifndef MYGIMP_UTILS_HPP
#define MYGIMP_UTILS_HPP


#include <string>
#include <vector>
#include <filesystem>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#ifndef _WIN32
#define MB_ICONERROR 1
#else
#include <windows.h>
#endif


namespace Utils
{
	std::string toUpper(std::string str);
	std::string toLower(std::string str);

	//! @brief Display a Windows dialog box.
	//! @details This functions opens a Windows dialog box and return the button clicked by the user.
	//! @param title The title of the window.
	//! @param content The content of the box.
	//! @param variate A bit combination of the window attributes (see Windows MessageBox function for a list of the enums).
	//! @return The button clicked by the user.
	//! @note On Non-Windows systems, it will simulate the Windows dialog box. Only MB_ICONERROR and MB_OK are simulated on those systems.
	int	dispMsg(const std::string &title, const std::string &content, int variate);

	//! @brief Get the last Exception Name
	//! @details Return the last type of Exception name
	//! @return std::string The last Exception name
	std::string getLastExceptionName();

	//! @brief Opens a FileDialog
	//! @param title Title of the FileDialog
	//! @param basePath The path of the FileDialog
	//! @param patterns The patterns of the FileDialog
	//! @return std::string FileDialog message
	std::string openFileDialog(const std::string &title = "Open file", const std::string &basePath = ".", const std::vector<std::pair<std::string, std::string>> &patterns = {}, bool overWriteWarning = false, bool mustExist = true);

	//! @brief Saves a file dialog
	//! @param title Title of the FileDialog
	//! @param basePath The path of the FileDialog
	//! @param patterns The patterns of the FileDialog
	//! @return std::string FileDialog message
	std::string saveFileDialog(const std::string &title = "Save file", const std::string &basePath = ".", const std::vector<std::pair<std::string, std::string>> &patterns = {});

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

	//! @brief Display a focused window.
	//! @param gui The gui handling the window.
	//! @param width The width of the window.
	//! @param height The height of the window.
	//! @return A pointer to the window
	tgui::ChildWindow::Ptr openWindowWithFocus(tgui::Gui &gui, tgui::Layout width, tgui::Layout height);
}


#endif //MYGIMP_UTILS_HPP
