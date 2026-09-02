#include "visu/helpers/OpenExternal.hpp"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

void openInDefaultApp(const std::filesystem::path &_path)
{
    ShellExecuteW(nullptr, L"open", _path.wstring().c_str(),
                  nullptr, nullptr, SW_SHOWNORMAL);
}

void openInVSCode(const std::filesystem::path &_path)
{
    ShellExecuteW(nullptr, L"open", L"code", _path.wstring().c_str(),
                  nullptr, SW_SHOWNORMAL);
}

void launchProgram(const std::filesystem::path &_exePath)
{
    std::filesystem::path dir = _exePath.parent_path();
    ShellExecuteW(nullptr, L"open", _exePath.wstring().c_str(),
                  nullptr, dir.wstring().c_str(), SW_SHOWNORMAL);
}