#include "visu/helpers/OpenExternal.hpp"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

void openInDefaultApp(const std::filesystem::path &path)
{
    ShellExecuteW(nullptr, L"open", path.wstring().c_str(),
                  nullptr, nullptr, SW_SHOWNORMAL);
}

void launchProgram(const std::filesystem::path &exePath)
{
    std::filesystem::path dir = exePath.parent_path();
    ShellExecuteW(nullptr, L"open", exePath.wstring().c_str(),
                  nullptr, dir.wstring().c_str(), SW_SHOWNORMAL);
}