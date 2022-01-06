#if !defined(__SQLIGHT_UTF8__)
#define __SQLIGHT_UTF8__
#include <string>
#include <string_view>

constexpr auto MAX_CMD_LINE = 1024;

#ifdef _WIN32
//utf16le(unicode of windows) to utf8
std::string utf16ToUtf8(std::wstring_view utf16Str);
#endif

//Get a line from stdin with utf8 encoding
std::string getLineUtf8();

#endif // __SQLIGHT_UTF8__
