#include "stdafx.h"

std::wstring ToWstring(const std::string &str)
{
    std::wstring wstr;
    wstr.assign(str.begin(), str.end());
    return wstr;
}

std::string ToString(const std::wstring &wstr)
{
    std::string str;
    str.assign(wstr.begin(), wstr.end());
    return str;
}
