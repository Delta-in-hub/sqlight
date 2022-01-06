
#include "utf8.h"
#include <iostream>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <winnls.h>
#include <winbase.h>

//utf16le(unicode of windows) to utf8
std::string utf16ToUtf8(std::wstring_view utf16Str)
{
    int buffLen = WideCharToMultiByte(CP_UTF8, 0, utf16Str.data(), utf16Str.size(), NULL, 0,
                                      /*// For the CP_UTF7 and CP_UTF8 settings for CodePage, this parameter must be set to NULL.*/
                                      NULL, NULL);
    std::string utf8Str(buffLen, 0);
    WideCharToMultiByte(CP_UTF8, 0, utf16Str.data(), utf16Str.size(), utf8Str.data(), buffLen, NULL, NULL);
    return utf8Str;
}
#endif

//Get a line from stdin with utf8 encoding
std::string getLineUtf8()
{
#if defined _WIN32
    wchar_t buffer[MAX_CMD_LINE] = {};
    DWORD numRead = 0;
    ReadConsoleW(GetStdHandle(STD_INPUT_HANDLE), buffer, MAX_CMD_LINE, &numRead, NULL);
    if (numRead >= MAX_CMD_LINE)
    {
        //
    }
    std::string utf8str = utf16ToUtf8(buffer);
    int len = utf8str.size();
    if (len >= 2 and utf8str[len - 2] == 0x0D and utf8str[len - 1] == 0x0A) //remove cr lf
    {
        utf8str.pop_back();
        utf8str.pop_back();
    }
    return utf8str;
#else
    std::string s;
    std::getline(std::cin, s);
    return s;
#endif
}
